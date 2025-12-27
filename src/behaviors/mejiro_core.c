/*
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "mejiro/mejiro_core.h"
#include "mejiro/mejiro_key_ids.h"
#include "mejiro/mejiro_tables.h"

/* 送信は roman 実装へ */
#include "mejiro/mejiro_send_roman.h"

LOG_MODULE_REGISTER(mejiro_core, CONFIG_ZMK_LOG_LEVEL);

/* ---- helpers ---------------------------------------------------------- */

static inline void append_char(char *out, size_t out_len, size_t *pos, char c) {
    if (*pos + 1 >= out_len) {
        return;
    }
    out[*pos] = c;
    (*pos)++;
    out[*pos] = '\0';
}

/* key order: s t k N n y i a U */
static const struct {
    uint32_t bit;
    char ch;
} mj_order[] = {
    { 1u << 0, 's' }, /* S */
    { 1u << 1, 't' }, /* T */
    { 1u << 2, 'k' }, /* K */
    { 1u << 3, 'N' }, /* N */
    { 1u << 4, 'n' }, /* n */
    { 1u << 5, 'y' }, /* Y */
    { 1u << 6, 'i' }, /* I */
    { 1u << 7, 'a' }, /* A */
    { 1u << 8, 'U' }, /* U */
};

void mejiro_state_reset(struct mejiro_state *s) {
    if (!s) return;
    s->left_mask = 0;
    s->right_mask = 0;
    s->mod_mask = 0;
    s->left_latched = 0;
    s->right_latched = 0;
    s->mod_latched = 0;
    s->active = false;
}

static inline bool is_left_id(uint32_t id) { return id <= MJ_L_U; }
static inline bool is_right_id(uint32_t id) { return (id >= MJ_R_S) && (id <= MJ_R_U); }

void mejiro_state_set_key(struct mejiro_state *s, uint32_t key_id, bool pressed) {
    if (!s) return;

    if (is_left_id(key_id)) {
        uint32_t bit = 1u << key_id; /* 0..8 */
        if (pressed) s->left_mask |= bit;
        else s->left_mask &= ~bit;
        return;
    }

    if (is_right_id(key_id)) {
        uint32_t bit = 1u << (key_id - MJ_R_S); /* 0..8 */
        if (pressed) s->right_mask |= bit;
        else s->right_mask &= ~bit;
        return;
    }

    if (key_id == MJ_H) {
        uint32_t bit = 1u << 0;
        if (pressed) s->mod_mask |= bit;
        else s->mod_mask &= ~bit;
        return;
    }

    if (key_id == MJ_X) {
        uint32_t bit = 1u << 1;
        if (pressed) s->mod_mask |= bit;
        else s->mod_mask &= ~bit;
        return;
    }
}

bool mejiro_build_stroke_string(const struct mejiro_state *latched, char *out, size_t out_len) {
    if (!latched || !out || out_len == 0) return false;
    out[0] = '\0';

    const uint32_t L = latched->left_mask;
    const uint32_t R = latched->right_mask;
    const uint32_t M = latched->mod_mask;

    /* 左文字列 */
    char left[32];
    char right[32];
    size_t lp = 0, rp = 0;
    left[0] = '\0';
    right[0] = '\0';

    for (size_t i = 0; i < (sizeof(mj_order) / sizeof(mj_order[0])); i++) {
        if (L & mj_order[i].bit) append_char(left, sizeof(left), &lp, mj_order[i].ch);
    }

    /* H/X は「左右で分けない」ので左側に付ける（仕様の最小解釈） */
    if (M & (1u << 0)) append_char(left, sizeof(left), &lp, '#');
    if (M & (1u << 1)) append_char(left, sizeof(left), &lp, '*');

    for (size_t i = 0; i < (sizeof(mj_order) / sizeof(mj_order[0])); i++) {
        if (R & mj_order[i].bit) append_char(right, sizeof(right), &rp, mj_order[i].ch);
    }

    /* 出力規則 */
    if (lp == 0 && rp == 0) {
        return false; /* 空 */
    }

    size_t op = 0;
    out[0] = '\0';

    if (lp > 0) {
        for (size_t i = 0; i < lp; i++) append_char(out, out_len, &op, left[i]);
    }

    if (rp > 0) {
        /* 右だけなら先頭 '-' */
        if (lp == 0) append_char(out, out_len, &op, '-');
        else append_char(out, out_len, &op, '-');

        for (size_t i = 0; i < rp; i++) append_char(out, out_len, &op, right[i]);
    }

    return true;
}

bool mejiro_try_emit(const struct mejiro_state *latched, int64_t timestamp) {
    char stroke[64];
    char out[96];

    if (!mejiro_build_stroke_string(latched, stroke, sizeof(stroke))) {
        return false;
    }

    /* ここがあなたの確認ポイント: stroke が期待通り出るか */
    LOG_DBG("MEJIRO stroke='%s' (L=0x%08x R=0x%08x M=0x%08x)",
            stroke, latched->left_mask, latched->right_mask, latched->mod_mask);

    if (!mejiro_tables_lookup(stroke, out, sizeof(out))) {
        LOG_DBG("MEJIRO tables: no match for '%s'", stroke);
        return false;
    }

    LOG_DBG("MEJIRO emit: '%s' <= '%s'", out, stroke);
    return mejiro_send_text(out, timestamp);
}
