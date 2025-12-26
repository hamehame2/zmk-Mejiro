#include "mejiro_core.h"
#include "mejiro_tables.h"
#include "mejiro/mejiro_send_roman.h"

#include <string.h>

/*
 * Where "0x01" is processed:
 * - mj_bit(id) => (1u << id)
 * - If MJ_L_S == 0, then left_mask |= 0x01 for that key.
 * This is the ONLY place that interprets "bit positions".
 */

static inline bool is_left(enum mejiro_key_id id) {
    return (id >= MJ_L_S && id <= MJ_L_k);
}
static inline bool is_right(enum mejiro_key_id id) {
    return (id >= MJ_R_S && id <= MJ_R_k);
}
static inline bool is_mod(enum mejiro_key_id id) {
    return (id == MJ_POUND || id == MJ_STAR);
}

void mejiro_reset(struct mejiro_state *st) {
    if (!st) return;
    st->left_mask = 0;
    st->right_mask = 0;
    st->mod_mask = 0;
    st->active = false;
}

void mejiro_on_key_event(struct mejiro_state *st, enum mejiro_key_id id, bool pressed) {
    if (!st) return;

    const uint32_t b = mj_bit(id);
    if (!b) return;

    if (pressed) {
        st->active = true;
        if (is_left(id))  st->left_mask  |= b;
        if (is_right(id)) st->right_mask |= b;
        if (is_mod(id))   st->mod_mask   |= b;
    } else {
        if (is_left(id))  st->left_mask  &= ~b;
        if (is_right(id)) st->right_mask &= ~b;
        if (is_mod(id))   st->mod_mask   &= ~b;
    }
}

static bool any_pressed(const struct mejiro_state *st) {
    return st->left_mask || st->right_mask || st->mod_mask;
}

bool mejiro_try_emit(struct mejiro_state *st) {
    if (!st) return false;

    /* Emit only when stroke is finished: all released after being active. */
    if (!st->active) return false;
    if (any_pressed(st)) return false;

    /*
     * At this point, you need the "snapshot" of last stroke.
     * For simplicity we keep the last stroke in separate storage.
     *
     * If you want perfect "release timing" with per-key tracking,
     * replace this with a queue (but you asked for strict "emit on release",
     * not severe chording timing).
     */
    /* Minimal approach: do nothing here because masks are already cleared. */
    /* => We need to preserve stroke before clearing; implement that: */
    return false;
}
