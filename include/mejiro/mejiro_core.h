/*
 * SPDX-License-Identifier: MIT
 *
 * Mejiro core state + helpers.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mejiro_state {
    /* behavior_mejiro.c が期待している名前 */
    uint32_t left_mask;   /* bits for Left keys (0..8)  */
    uint32_t right_mask;  /* bits for Right keys (0..8) */
    uint32_t mod_mask;    /* bits for specials (H/X)    */

    /* optional (latched用などで使う) */
    uint32_t left_latched;
    uint32_t right_latched;
    uint32_t mod_latched;

    bool active;
};

/* Reset a state (all masks -> 0) */
void mejiro_state_reset(struct mejiro_state *s);

/* Update state by key-id press/release */
void mejiro_state_set_key(struct mejiro_state *s, uint32_t key_id, bool pressed);

/*
 * Build stroke string from a latched state.
 * Rules (あなたが書いた仕様に合わせた最小実装):
 * - left only: "tk#" など（右が無ければ '-' を入れない）
 * - right only: "-t" / "-U" など（右だけは '-' で始める）
 * - both: "stk-..." のように '-' を挟む
 * - H => '#', X => '*'
 */
bool mejiro_build_stroke_string(const struct mejiro_state *latched, char *out, size_t out_len);

/* Try emit (tables lookup + roman sender). Return true if emitted. */
bool mejiro_try_emit(const struct mejiro_state *latched, int64_t timestamp);

#ifdef __cplusplus
}
#endif
