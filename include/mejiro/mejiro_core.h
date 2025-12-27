#pragma once
/*
 * SPDX-License-Identifier: MIT
 *
 * Mejiro core state + helpers
 *
 * IMPORTANT:
 * - This file MUST define `struct mejiro_state`.
 * - Do NOT replace this header with key-id enums etc.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * State model (your current approach):
 * - pressed_left/right : current down keys (bitmask)
 * - latched_left/right : accumulated keys participating in the stroke while any key is down
 * - active             : whether we are currently tracking a stroke
 */
struct mejiro_state {
    bool active;

    uint32_t pressed_left;
    uint32_t pressed_right;

    uint32_t latched_left;
    uint32_t latched_right;
};

/* basic state ops */
void mejiro_reset(struct mejiro_state *s);
void mejiro_set_active(struct mejiro_state *s, bool on);

/*
 * Key events:
 * - key_id is your logical key id (enum mejiro_key_id etc.)
 * - is_left selects whether the key belongs to left or right side bitfield
 *
 * return: true if accepted/handled
 */
bool mejiro_on_key_press(struct mejiro_state *s, uint16_t key_id, bool is_left, int64_t timestamp);
bool mejiro_on_key_release(struct mejiro_state *s, uint16_t key_id, bool is_left, int64_t timestamp);

/*
 * Try emit:
 * - should be called when you detect "all released" timing.
 * - builds stroke string and dictionary lookup + output (mejiro_tables + mejiro_send_roman).
 */
bool mejiro_try_emit(struct mejiro_state *s, int64_t timestamp);

#ifdef __cplusplus
}
#endif
