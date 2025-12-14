/*
 * mejiro_core.c
 *
 * Mejiro core logic (stroke normalization + stroke_to_kana + joshi).
 * - Uses ZMK keycode symbols (Q, W, A..Z, COMMA, DOT, SLASH, SEMI, etc.)
 *   from <dt-bindings/zmk/keys.h>.
 * - Does NOT use QMK-style KC_*.
 *
 * You typically place this file next to behavior_naginata.c:
 *   <your zmk module>/src/behaviors/mejiro_core.c
 */

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>




#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <stdbool.h>

#include <dt-bindings/zmk/keys.h>
#include <zmk_naginata/nglist.h>
#include <zmk_naginata/naginata_func.h> /* for nglist types (and any shared helpers) */

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* ==============================
 *  Mejiro common structures
 * ============================== */

typedef struct {
    char left_conso[5];     // left STKN
    char left_vowel[5];     // left YIAU
    char left_particle[4];  // left ntk
    char sep;               // '-', '#', or '\0'
    char right_conso[5];    // right STKN
    char right_vowel[5];    // right YIAU
    char right_particle[4]; // right ntk
    bool star;              // '*' pressed?
} mej_stroke_t;

typedef struct {
    const char *cv;         // base kana
    const char *suffix;     // diphthong suffix
    const char *extra;      // SECOND_SOUND
    char        conso_roma; // row consonant
    const char *vowel_roma; // vowel romaji
    int         vowel_index;
} mej_kana_t;

/* ==============================
 *  Physical key -> Mejiro symbol
 * ============================== */

typedef enum { MEJ_SIDE_NONE=0, MEJ_SIDE_LEFT, MEJ_SIDE_RIGHT } mej_side_t;
typedef enum { MEJ_KIND_NONE=0, MEJ_KIND_CONSO, MEJ_KIND_VOWEL, MEJ_KIND_PART, MEJ_KIND_HASH, MEJ_KIND_STAR } mej_kind_t;

typedef struct {
    uint32_t   keycode;
    mej_side_t side;
    mej_kind_t kind;
    char       symbol; // 'S','T','K','N','Y','I','A','U','n','t','k','#','*'
} mej_key_map_t;

/*
 * Default mapping matches your spreadsheet (QWERTY positions):
 *
 * Left:  Q W E R T   -> # T Y I U
 *        A S D F G   -> S K N A U
 *        C V B       -> n t k   (thumbs)
 *
 * Right: Y U I O P   -> U I Y T *
 *        H J K L ;   -> U A N K S
 *        N M ,       -> k t n   (thumbs)
 *
 * If your actual matrix differs, edit this table only.
 */
static const mej_key_map_t mej_key_map[] = {
    /* Left top */
    { Q,     MEJ_SIDE_LEFT,  MEJ_KIND_HASH,  '#' },
    { W,     MEJ_SIDE_LEFT,  MEJ_KIND_CONSO, 'T' },
    { E,     MEJ_SIDE_LEFT,  MEJ_KIND_VOWEL, 'Y' },
    { R,     MEJ_SIDE_LEFT,  MEJ_KIND_VOWEL, 'I' },
    { T,     MEJ_SIDE_LEFT,  MEJ_KIND_VOWEL, 'U' },

    /* Left mid */
    { A,     MEJ_SIDE_LEFT,  MEJ_KIND_CONSO, 'S' },
    { S,     MEJ_SIDE_LEFT,  MEJ_KIND_CONSO, 'K' },
    { D,     MEJ_SIDE_LEFT,  MEJ_KIND_CONSO, 'N' },
    { F,     MEJ_SIDE_LEFT,  MEJ_KIND_VOWEL, 'A' },
    { G,     MEJ_SIDE_LEFT,  MEJ_KIND_VOWEL, 'U' },

    /* Left thumbs */
    { C,     MEJ_SIDE_LEFT,  MEJ_KIND_PART,  'n' },
    { V,     MEJ_SIDE_LEFT,  MEJ_KIND_PART,  't' },
    { B,     MEJ_SIDE_LEFT,  MEJ_KIND_PART,  'k' },

    /* Right top */
    { Y,     MEJ_SIDE_RIGHT, MEJ_KIND_VOWEL, 'U' },
    { U,     MEJ_SIDE_RIGHT, MEJ_KIND_VOWEL, 'I' },
    { I,     MEJ_SIDE_RIGHT, MEJ_KIND_VOWEL, 'Y' },
    { O,     MEJ_SIDE_RIGHT, MEJ_KIND_CONSO, 'T' },
    { P,     MEJ_SIDE_RIGHT, MEJ_KIND_STAR,  '*' },

    /* Right mid */
    { H,     MEJ_SIDE_RIGHT, MEJ_KIND_VOWEL, 'U' },
    { J,     MEJ_SIDE_RIGHT, MEJ_KIND_VOWEL, 'A' },
    { K,     MEJ_SIDE_RIGHT, MEJ_KIND_CONSO, 'N' },
    { L,     MEJ_SIDE_RIGHT, MEJ_KIND_CONSO, 'K' },
    { SEMI,  MEJ_SIDE_RIGHT, MEJ_KIND_CONSO, 'S' },

    /* Right thumbs */
    { N,     MEJ_SIDE_RIGHT, MEJ_KIND_PART,  'k' },
    { M,     MEJ_SIDE_RIGHT, MEJ_KIND_PART,  't' },
    { COMMA, MEJ_SIDE_RIGHT, MEJ_KIND_PART,  'n' },
};

static const mej_key_map_t *mej_find_key(uint32_t kc) {
    for (size_t i = 0; i < ARRAY_SIZE(mej_key_map); i++) {
        if (mej_key_map[i].keycode == kc) return &mej_key_map[i];
    }
    return NULL;
}

static const char ORDER_CONSO[]    = "STKN";
static const char ORDER_VOWEL[]    = "YIAU";
static const char ORDER_PARTICLE[] = "ntk";

static void mej_build_stroke_from_keys(const NGList *keys, mej_stroke_t *out) {
    memset(out, 0, sizeof(*out));

    bool left_has  = false;
    bool right_has = false;

    bool lc[4] = {0}, lv[4] = {0}, lp[3] = {0};
    bool rc[4] = {0}, rv[4] = {0}, rp[3] = {0};

    for (int i = 0; i < keys->size; i++) {
        uint32_t kc = keys->elements[i];
        const mej_key_map_t *m = mej_find_key(kc);
        if (!m) continue;

        if (m->kind == MEJ_KIND_HASH) { out->sep = '#'; left_has = true; continue; }
        if (m->kind == MEJ_KIND_STAR) { out->star = true; right_has = true; continue; }

        int idx;
        switch (m->kind) {
        case MEJ_KIND_CONSO:
            idx = (int)(strchr(ORDER_CONSO, m->symbol) - ORDER_CONSO);
            if (idx >= 0 && idx < 4) { if (m->side == MEJ_SIDE_LEFT) lc[idx]=true; else rc[idx]=true; }
            break;
        case MEJ_KIND_VOWEL:
            idx = (int)(strchr(ORDER_VOWEL, m->symbol) - ORDER_VOWEL);
            if (idx >= 0 && idx < 4) { if (m->side == MEJ_SIDE_LEFT) lv[idx]=true; else rv[idx]=true; }
            break;
        case MEJ_KIND_PART:
            idx = (int)(strchr(ORDER_PARTICLE, m->symbol) - ORDER_PARTICLE);
            if (idx >= 0 && idx < 3) { if (m->side == MEJ_SIDE_LEFT) lp[idx]=true; else rp[idx]=true; }
            break;
        default:
            break;
        }

        if (m->side == MEJ_SIDE_LEFT)  left_has  = true;
        if (m->side == MEJ_SIDE_RIGHT) right_has = true;
    }

    if (!out->sep) out->sep = (left_has && right_has) ? '-' : '\0';

    int p=0; for (int i=0;i<4;i++) if (lc[i]) out->left_conso[p++]=ORDER_CONSO[i]; out->left_conso[p]='\0';
    p=0; for (int i=0;i<4;i++) if (lv[i]) out->left_vowel[p++]=ORDER_VOWEL[i]; out->left_vowel[p]='\0';
    p=0; for (int i=0;i<3;i++) if (lp[i]) out->left_particle[p++]=ORDER_PARTICLE[i]; out->left_particle[p]='\0';

    p=0; for (int i=0;i<4;i++) if (rc[i]) out->right_conso[p++]=ORDER_CONSO[i]; out->right_conso[p]='\0';
    p=0; for (int i=0;i<4;i++) if (rv[i]) out->right_vowel[p++]=ORDER_VOWEL[i]; out->right_vowel[p]='\0';
    p=0; for (int i=0;i<3;i++) if (rp[i]) out->right_particle[p++]=ORDER_PARTICLE[i]; out->right_particle[p]='\0';
}

/* ==============================
 *  stroke_to_kana tables
 * ============================== */

typedef struct { const char *stroke; char roma; } mej_conso_t;
static const mej_conso_t MEJ_CONSO_ROMA[] = {
    { "",    '\0' }, { "S",'s' }, { "T",'t' }, { "K",'k' }, { "N",'n' },
    { "ST",'r' }, { "SK",'w' }, { "TK",'h' }, { "SN",'z' }, { "TN",'d' },
    { "KN",'g' }, { "TKN",'b' }, { "STK",'p' }, { "STN",'l' }, { "SKN",'m' }, { "STKN",'f' },
};

typedef struct { const char *stroke; const char *roma; } mej_vowel_t;
static const mej_vowel_t MEJ_VOWEL_ROMA[] = {
    { "A","a" }, { "I","i" }, { "U","u" }, { "IA","e" }, { "AU","o" }, { "YA","ya" }, { "YU","yu" }, { "YAU","yo" },
};

typedef struct { const char *stroke; const char *roma1; const char *suffix; } mej_dip_t;
static const mej_dip_t MEJ_DIPHTHONG[] = {
    { "Y","a","い" }, { "YI","yo","う" }, { "YIA","e","い" }, { "YIU","yu","う" }, { "YIAU","u","う" }, { "IU","u","い" }, { "IAU","o","う" },
};

typedef struct { const char *stroke; const char *roma1; const char *suffix; } mej_cdip_t;
static const mej_cdip_t MEJ_COMPLEX_DIP[] = {
    { "IAUn","o","お" }, { "YIn","i","い" }, { "YIUn","a","え" }, { "YIAUn","o","い" },
    { "IAUntk","ya","う" }, { "YIntk","ya","い" }, { "YIUntk","yu","い" }, { "YIAUntk","yo","い" },
};

static const char *MEJ_ROMA_TO_KANA[][8] = {
    /* '' */ { "あ","い","う","え","お","や","ゆ","よ" },
    /* k  */ { "か","き","く","け","こ","きゃ","きゅ","きょ" },
    /* s  */ { "さ","し","す","せ","そ","しゃ","しゅ","しょ" },
    /* t  */ { "た","ち","つ","て","と","ちゃ","ちゅ","ちょ" },
    /* n  */ { "な","に","ぬ","ね","の","にゃ","にゅ","にょ" },
    /* h  */ { "は","ひ","ふ","へ","ほ","ひゃ","ひゅ","ひょ" },
    /* m  */ { "ま","み","む","め","も","みゃ","みゅ","みょ" },
    /* r  */ { "ら","り","る","れ","ろ","りゃ","りゅ","りょ" },
    /* w  */ { "わ","うぃ","ゔ","うぇ","を","やあ","いう","うぉ" },
    /* g  */ { "が","ぎ","ぐ","げ","ご","ぎゃ","ぎゅ","ぎょ" },
    /* z  */ { "ざ","じ","ず","ぜ","ぞ","じゃ","じゅ","じょ" },
    /* d  */ { "だ","ぢ","づ","で","ど","でぃ","てぃ","どぅ" },
    /* b  */ { "ば","び","ぶ","べ","ぼ","びゃ","びゅ","びょ" },
    /* p  */ { "ぱ","ぴ","ぷ","ぺ","ぽ","ぴゃ","ぴゅ","ぴょ" },
    /* f  */ { "ふぁ","ふぃ","ふ","ふぇ","ふぉ","じぇ","しぇ","ちぇ" },
    /* l  */ { "ぁ","ぃ","ぅ","ぇ","ぉ","ゃ","ゅ","ょ" },
};

static const char MEJ_ROMA_ROWS[] = { '\0','k','s','t','n','h','m','r','w','g','z','d','b','p','f','l' };

static const char *MEJ_PARTICLE_KEYS[] = { "", "n","t","k","tk","nt","nk","ntk" };
static const char *MEJ_SECOND_SOUND[]  = { "", "ん","つ","く","っ","ち","き","ー" };

static char mej_last_vowel_stroke[8] = "A";

static int mej_find_particle_index(const char *stroke) {
    for (int i=0;i<8;i++) if (strcmp(MEJ_PARTICLE_KEYS[i], stroke)==0) return i;
    return 0;
}
static const mej_conso_t *mej_find_conso(const char *stroke) {
    for (size_t i=0;i<ARRAY_SIZE(MEJ_CONSO_ROMA);i++) if (strcmp(MEJ_CONSO_ROMA[i].stroke, stroke)==0) return &MEJ_CONSO_ROMA[i];
    return &MEJ_CONSO_ROMA[0];
}
static const mej_vowel_t *mej_find_vowel(const char *stroke) {
    for (size_t i=0;i<ARRAY_SIZE(MEJ_VOWEL_ROMA);i++) if (strcmp(MEJ_VOWEL_ROMA[i].stroke, stroke)==0) return &MEJ_VOWEL_ROMA[i];
    return NULL;
}
static int mej_row_index(char roma_row) {
    for (size_t i=0;i<ARRAY_SIZE(MEJ_ROMA_ROWS);i++) if (MEJ_ROMA_ROWS[i]==roma_row) return (int)i;
    return 0;
}

static bool mej_stroke_to_kana(const char *conso_stroke,
                               const char *vowel_stroke,
                               const char *particle_stroke,
                               bool asterisk,
                               mej_kana_t *out) {
    memset(out, 0, sizeof(*out));

    const char *current_vowel =
        (vowel_stroke && vowel_stroke[0]) ? vowel_stroke :
        (mej_last_vowel_stroke[0] ? mej_last_vowel_stroke : "A");
    strncpy(mej_last_vowel_stroke, current_vowel, sizeof(mej_last_vowel_stroke)-1);

    const char *v_roma = NULL;
    const char *suffix = "";
    const char *extra  = "";

    char buf[16];
    snprintf(buf, sizeof(buf), "%s%s", current_vowel, (particle_stroke ? particle_stroke : ""));

    if (!asterisk) {
        for (size_t i=0;i<ARRAY_SIZE(MEJ_COMPLEX_DIP);i++) {
            if (strcmp(MEJ_COMPLEX_DIP[i].stroke, buf)==0) {
                v_roma = MEJ_COMPLEX_DIP[i].roma1;
                suffix = MEJ_COMPLEX_DIP[i].suffix;
                extra  = "";
                break;
            }
        }
    }

    if (!v_roma) {
        for (size_t i=0;i<ARRAY_SIZE(MEJ_DIPHTHONG);i++) {
            if (strcmp(MEJ_DIPHTHONG[i].stroke, current_vowel)==0) {
                v_roma = MEJ_DIPHTHONG[i].roma1;
                suffix = MEJ_DIPHTHONG[i].suffix;
                extra  = MEJ_SECOND_SOUND[mej_find_particle_index(particle_stroke ? particle_stroke : "")];
                break;
            }
        }
    }

    int vowel_index = -1;
    if (!v_roma) {
        const mej_vowel_t *vt = mej_find_vowel(current_vowel);
        if (!vt) return false;
        v_roma = vt->roma;
        suffix = "";
        extra  = MEJ_SECOND_SOUND[mej_find_particle_index(particle_stroke ? particle_stroke : "")];

        for (size_t i=0;i<ARRAY_SIZE(MEJ_VOWEL_ROMA);i++) {
            if (strcmp(MEJ_VOWEL_ROMA[i].stroke, current_vowel)==0) { vowel_index=(int)i; break; }
        }
    } else {
        for (size_t i=0;i<ARRAY_SIZE(MEJ_VOWEL_ROMA);i++) {
            if (strcmp(MEJ_VOWEL_ROMA[i].roma, v_roma)==0) { vowel_index=(int)i; break; }
        }
    }
    if (vowel_index < 0) return false;

    const mej_conso_t *ct = mej_find_conso(conso_stroke ? conso_stroke : "");
    char c_roma = ct->roma;
    int row_idx = mej_row_index(c_roma);

    out->cv         = MEJ_ROMA_TO_KANA[row_idx][vowel_index];
    out->suffix     = suffix;
    out->extra      = extra;
    out->conso_roma = c_roma;
    out->vowel_roma = v_roma;
    out->vowel_index= vowel_index;
    return true;
}

/* joshi tables */

static const char *MEJ_L_PARTICLE[] = { "", "、", "に", "の", "で", "と", "を", "か" };
static const char *MEJ_R_PARTICLE[] = { "", "、", "は", "が", "も", "は、", "が、", "も、" };

typedef struct { const char *stroke; const char *text; } mej_exc_part_t;
static const mej_exc_part_t MEJ_EXC_PARTICLE[] = {
    { "-n",   "}{#Return}{" }, { "n-", "}{#Space}{" }, { "n-n","}{#Space}{#Return}{" },
    { "-nt",  "。" }, { "-nk", "、" }, { "-ntk","や" },
    { "n-nt", "?" }, { "n-nk","!" }, { "n-ntk","や、" },
};

static bool mej_joshi(const char *lp, const char *rp, char *out, size_t out_size) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%s-%s", (lp?lp:""), (rp?rp:""));

    for (size_t i=0;i<ARRAY_SIZE(MEJ_EXC_PARTICLE);i++) {
        if (strcmp(MEJ_EXC_PARTICLE[i].stroke, buf)==0) {
            strlcpy(out, MEJ_EXC_PARTICLE[i].text, out_size);
            return true;
        }
    }

    const char *rp_tk;
    const char *p_n = rp ? strchr(rp, 'n') : NULL;
    rp_tk = p_n ? p_n + 1 : (rp ? rp : "");

    int l_idx = mej_find_particle_index(lp ? lp : "");
    int r_idx = mej_find_particle_index(rp_tk);

    const char *lj = MEJ_L_PARTICLE[l_idx];
    const char *rj = MEJ_R_PARTICLE[r_idx];

    char tmp[32] = {0};

    if (lp && strcmp(lp, "n")==0) {
        snprintf(tmp, sizeof(tmp), "%s%s", rj, "、");
    } else if ((rp && (strcmp(rp, "k")==0 || strcmp(rp, "nk")==0)) &&
               !(lp==NULL || strcmp(lp,"")==0 || strcmp(lp,"k")==0)) {
        snprintf(tmp, sizeof(tmp), "の%s", lj);
        if (rp && strchr(rp,'n')) strlcat(tmp, "、", sizeof(tmp));
    } else {
        snprintf(tmp, sizeof(tmp), "%s%s", lj, rj);
        if (rp && strchr(rp,'n')) strlcat(tmp, "、", sizeof(tmp));
    }

    strlcpy(out, tmp, out_size);
    return out[0] != '\0';
}

/* Output hook: override in behavior_naginata.c if you want real key output */
//__attribute__((weak))
//void mej_output_utf8(const char *s) {
//    LOG_DBG("Mejiro output: %s", s);
//}

// mejiro_core.c の weak 関数を上書き
void mej_output_utf8(const char *s) {
    // ★ Mejiro が呼ばれたら必ず "q" を 1 回出す
    int64_t timestamp = k_uptime_get();
    raise_zmk_keycode_state_changed_from_encoded(Q, true, timestamp);
    raise_zmk_keycode_state_changed_from_encoded(Q, false, timestamp);

    LOG_INF("MEJIRO FIRED: %s", s);
}


/* Public entry: process one stroke (NGList) */
bool mej_type_once(const NGList *keys) {
    mej_stroke_t st;
    mej_build_stroke_from_keys(keys, &st);

    mej_kana_t left = {0}, right = {0};

    bool ok_l = mej_stroke_to_kana(st.left_conso, st.left_vowel, st.left_particle, st.star, &left);
    bool ok_r = mej_stroke_to_kana(st.right_conso, st.right_vowel, st.right_particle, st.star, &right);

    char joshi_buf[32] = {0};
    bool has_joshi = mej_joshi(st.left_particle, st.right_particle, joshi_buf, sizeof(joshi_buf));

    char out[128] = {0};

    if (ok_l && left.cv && left.cv[0]) {
        strlcat(out, left.cv, sizeof(out));
        if (left.suffix) strlcat(out, left.suffix, sizeof(out));
        if (left.extra)  strlcat(out, left.extra, sizeof(out));
    }
    if (ok_r && right.cv && right.cv[0]) {
        strlcat(out, right.cv, sizeof(out));
        if (right.suffix) strlcat(out, right.suffix, sizeof(out));
        if (right.extra)  strlcat(out, right.extra, sizeof(out));
    }
    if (has_joshi) {
        strlcat(out, joshi_buf, sizeof(out));
    }

    if (out[0]) {
        mej_output_utf8(out);
        return true;
    }
    return false;
}
