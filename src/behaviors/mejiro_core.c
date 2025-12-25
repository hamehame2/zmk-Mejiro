/* -------------------------------------------------------------------------
 * FILE: src/behaviors/mejiro_core.c
 * ------------------------------------------------------------------------- */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <mejiro/mejiro_send_roman.h>

/* tables */
#include <mejiro/mejiro_tables.h>

LOG_MODULE_REGISTER(mejiro_core, CONFIG_ZMK_LOG_LEVEL);

/* pressed now (currently held) */
static uint32_t held_mask;
/* keys that participated since last commit (union of presses) */
static uint32_t chord_mask;

/* --- key->stroke mapping (暫定: 7key を Mejiro "S T N K" 系へ落とす) ---
 * 目的:
 *  1) 「全キー離した瞬間に1回だけ出す」骨格を固める
 *  2) tables(=mj_commands) に当たる文字列を生成できるようにする
 *
 * ここは後であなたの本命仕様に合わせて差し替える前提。
 * まずは実害が少ないよう、未知の組み合わせは何も出さない。
 *
 * 現状の生成規則:
 *  - 右手側のキーが1つでも入っていたら先頭に '-' を付ける（tables の "-SKNA" 形式に合わせる）
 *  - H:  S
 *  - T:  TL/TR -> T
 *  - N:  NL/NR -> N
 *  - K:  KL/KR -> K
 *
 * 例:
 *  - H + KL + NL + TR + NR  -> "-SKNT"
 *    ※並び順は S K N T の固定。必要なら変更。
 */
static bool build_stroke_from_mask(uint32_t m, char *out, size_t out_sz) {
    if (!out || out_sz < 2) {
        return false;
    }

    /* key ids are 0..6 */
    const bool has_h  = (m & (1u << 0)) != 0; /* MJ_H  */
    const bool has_tl = (m & (1u << 1)) != 0; /* MJ_TL */
    const bool has_nl = (m & (1u << 2)) != 0; /* MJ_NL */
    const bool has_tr = (m & (1u << 3)) != 0; /* MJ_TR */
    const bool has_nr = (m & (1u << 4)) != 0; /* MJ_NR */
    const bool has_kl = (m & (1u << 5)) != 0; /* MJ_KL */
    const bool has_kr = (m & (1u << 6)) != 0; /* MJ_KR */

    const bool right = has_tr || has_nr || has_kr;

    /* If nothing meaningful -> no stroke */
    if (!(has_h || has_tl || has_nl || has_tr || has_nr || has_kl || has_kr)) {
        out[0] = '\0';
        return false;
    }

    size_t p = 0;

    if (right) {
        if (p + 1 >= out_sz) return false;
        out[p++] = '-';
    }

    /* fixed order to match existing tables examples (-SKNA etc).
     * (暫定) S K N T の順。必要なら後であなたの期待通りに変更。
     */
    if (has_h) {
        if (p + 1 >= out_sz) return false;
        out[p++] = 'S';
    }

    if (has_kl || has_kr) {
        if (p + 1 >= out_sz) return false;
        out[p++] = 'K';
    }

    if (has_nl || has_nr) {
        if (p + 1 >= out_sz) return false;
        out[p++] = 'N';
    }

    if (has_tl || has_tr) {
        if (p + 1 >= out_sz) return false;
        out[p++] = 'T';
    }

    out[p] = '\0';
    return p > 0;
}

static const struct mj_kv *find_kv(const struct mj_kv *arr, size_t n, const char *key) {
    if (!arr || !key) return NULL;
    for (size_t i = 0; i < n; i++) {
        if (arr[i].k && arr[i].v && (strcmp(arr[i].k, key) == 0)) {
            return &arr[i];
        }
    }
    return NULL;
}

static void commit_if_ready(void) {
    if (held_mask != 0) {
        return; /* still holding something */
    }

    if (chord_mask == 0) {
        return;
    }

    char stroke[32];
    const bool ok = build_stroke_from_mask(chord_mask, stroke, sizeof(stroke));

    LOG_DBG("commit: mask=0x%08x stroke='%s'", chord_mask, ok ? stroke : "<none>");

    chord_mask = 0;

    if (!ok || stroke[0] == '\0') {
        return;
    }

    /* Lookup order: users -> abstract -> verbs -> commands (今は commands だけ実体あり) */
    const struct mj_kv *kv = NULL;
    kv = find_kv(mj_users, mj_users_len, stroke);
    if (!kv) kv = find_kv(mj_abstract, mj_abstract_len, stroke);
    if (!kv) kv = find_kv(mj_verbs, mj_verbs_len, stroke);
    if (!kv) kv = find_kv(mj_commands, mj_commands_len, stroke);

    if (!kv) {
        LOG_DBG("no table hit for '%s'", stroke);
        return;
    }

    (void)mejiro_send_roman_exec(kv->v);
}

void mejiro_core_on_press(uint8_t key_id) {
    if (key_id >= 32) {
        return;
    }
    const uint32_t bit = 1u << key_id;
    held_mask |= bit;
    chord_mask |= bit;
}

void mejiro_core_on_release(uint8_t key_id) {
    if (key_id >= 32) {
        return;
    }
    const uint32_t bit = 1u << key_id;
    held_mask &= ~bit;
    commit_if_ready();
}

void mejiro_core_reset(void) {
    held_mask = 0;
    chord_mask = 0;
}
