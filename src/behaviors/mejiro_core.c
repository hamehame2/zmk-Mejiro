/*
 * Mejiro core: build stroke string from held bitmask and emit roman string.
 * Also provides compatibility/wrapper symbols required by behavior_mejiro.c
 * and behavior_naginata.c, without adding any extra files.
 */

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/keymap.h>

/* for stub: mejiro_try_emit_from_nginput */
#include <zmk_naginata/nglistarray.h>

#include "mejiro/mejiro_core.h"
#include "mejiro/mejiro_tables.h"
#include "mejiro/mejiro_send_roman.h"

/* IDs are defined in dt-bindings/zmk/mejiro.h */
#include <dt-bindings/zmk/mejiro.h>

LOG_MODULE_REGISTER(mejiro_core, CONFIG_ZMK_LOG_LEVEL);

/* --- internal state --------------------------------------------------- */
static uint32_t held_mask = 0;  /* keys currently held */
static uint32_t chord_mask = 0; /* union of keys pressed in this chord */

/* --- helpers ---------------------------------------------------------- */
static inline bool id_valid(uint8_t id) { return id <= MJ_X; }
static inline uint32_t bit(uint8_t id) { return (uint32_t)1u << id; }

static const struct mj_kv *find_kv(const struct mj_kv *tbl, size_t len, const char *k) {
    if (!tbl || !k) {
        return NULL;
    }
    for (size_t i = 0; i < len; i++) {
        if (tbl[i].k && strcmp(tbl[i].k, k) == 0) {
            return &tbl[i];
        }
    }
    return NULL;
}

/*
 * 原典(mejiro_base.py)の表記に合わせる：
 * - 左: STKNYIAUntk
 * - 右: STKNYIAUntk
 * - '-' は「右に何かある」or「左が空で右だけ」のとき必須
 * - '#' は中置（右があるなら -#...）、右が無いなら末尾（tk# / #）
 * - '*' は末尾
 */
static bool build_stroke_from_mask(uint32_t m, char *out, size_t out_sz) {
    if (!out || out_sz == 0) {
        return false;
    }
    out[0] = '\0';

    char L[16] = {0};
    char R[16] = {0};
    size_t li = 0, ri = 0;

    /* left order */
    if (m & bit(MJ_L_S)) L[li++] = 'S';
    if (m & bit(MJ_L_T)) L[li++] = 'T';
    if (m & bit(MJ_L_K)) L[li++] = 'K';
    if (m & bit(MJ_L_N)) L[li++] = 'N';
    if (m & bit(MJ_L_Y)) L[li++] = 'Y';
    if (m & bit(MJ_L_I)) L[li++] = 'I';
    if (m & bit(MJ_L_A)) L[li++] = 'A';
    if (m & bit(MJ_L_U)) L[li++] = 'U';
    if (m & bit(MJ_L_n)) L[li++] = 'n';
    if (m & bit(MJ_L_t)) L[li++] = 't';
    if (m & bit(MJ_L_k)) L[li++] = 'k';
    L[li] = '\0';

    /* right order */
    if (m & bit(MJ_R_S)) R[ri++] = 'S';
    if (m & bit(MJ_R_T)) R[ri++] = 'T';
    if (m & bit(MJ_R_K)) R[ri++] = 'K';
    if (m & bit(MJ_R_N)) R[ri++] = 'N';
    if (m & bit(MJ_R_Y)) R[ri++] = 'Y';
    if (m & bit(MJ_R_I)) R[ri++] = 'I';
    if (m & bit(MJ_R_A)) R[ri++] = 'A';
    if (m & bit(MJ_R_U)) R[ri++] = 'U';
    if (m & bit(MJ_R_n)) R[ri++] = 'n';
    if (m & bit(MJ_R_t)) R[ri++] = 't';
    if (m & bit(MJ_R_k)) R[ri++] = 'k';
    R[ri] = '\0';

    const bool has_left = (li > 0);
    const bool has_right = (ri > 0);
    const bool has_H = (m & bit(MJ_H)) != 0; /* # */
    const bool has_X = (m & bit(MJ_X)) != 0; /* * */

    /* '-' rules */
    const bool need_dash = has_right || (!has_left && has_right);

    size_t w = 0;

#define APPEND_CH(ch)                                                                              \
    do {                                                                                           \
        if (w + 1 >= out_sz)                                                                       \
            return false;                                                                          \
        out[w++] = (ch);                                                                           \
        out[w] = '\0';                                                                             \
    } while (0)

#define APPEND_STR(s)                                                                              \
    do {                                                                                           \
        const char *_s = (s);                                                                      \
        const size_t _n = strlen(_s);                                                              \
        if (w + _n >= out_sz)                                                                      \
            return false;                                                                          \
        memcpy(&out[w], _s, _n);                                                                   \
        w += _n;                                                                                   \
        out[w] = '\0';                                                                             \
    } while (0)

    if (has_left) {
        APPEND_STR(L);
    }

    if (need_dash) {
        APPEND_CH('-');
        /* 原典: '-' の後に # が来る形（-#...） */
        if (has_H)
            APPEND_CH('#');
        if (has_right)
            APPEND_STR(R);
    } else {
        /* 右が無いので # は末尾側に来る（tk# / #） */
        if (has_H)
            APPEND_CH('#');
    }

    if (has_X) {
        APPEND_CH('*');
    }

    if (w == 0) {
        return false;
    }
    return true;
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
    LOG_DBG("commit: mask=0x%08x stroke='%s'", chord_mask, ok ? stroke : "");
    chord_mask = 0;

    if (!ok || stroke[0] == '\0') {
        return;
    }

    /* Lookup order: users -> abstract -> verbs -> commands */
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

/* --- public API ------------------------------------------------------- */
void mejiro_on_press(uint8_t key_id) {
    if (!id_valid(key_id))
        return;
    const uint32_t b = bit(key_id);
    held_mask |= b;
    chord_mask |= b;
    LOG_DBG("press id=%u held=0x%08x chord=0x%08x", key_id, held_mask, chord_mask);
}

void mejiro_on_release(uint8_t key_id) {
    if (!id_valid(key_id))
        return;
    const uint32_t b = bit(key_id);
    held_mask &= ~b;
    LOG_DBG("release id=%u held=0x%08x chord=0x%08x", key_id, held_mask, chord_mask);
    commit_if_ready();
}

/* naginata 側などから「今すぐ commit」したい用 */
bool mejiro_commit_now(void) {
    if (held_mask != 0)
        return false;
    if (chord_mask == 0)
        return false;
    commit_if_ready();
    return true;
}

/* ----------------------------------------------------------------------
 * Compatibility / missing-symbol fillers (NO extra files)
 * -------------------------------------------------------------------- */

/* behavior_mejiro.c が参照するシンボル名に合わせたラッパ */
int mejiro_core_on_press(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    (void)event;
    /* &mj L_n などの param1 を Mejiro key_id として扱う想定 */
    mejiro_on_press((uint8_t)binding->param1);
    return 0;
}

int mejiro_core_on_release(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    (void)event;
    mejiro_on_release((uint8_t)binding->param1);
    return 0;
}

/* naginata_release から参照されるシンボルの穴埋め（あなた指定の stub） */
bool mejiro_try_emit_from_nginput(const NGListArray *nginput) {
    (void)nginput;
    return false;
}
