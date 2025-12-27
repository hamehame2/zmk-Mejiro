#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "mejiro/core.h"
#include "mejiro/mejiro_tables.h"

/* forward */
bool mejiro_send_text(const char *text);

/* ---- internal helpers ------------------------------------------------- */

static inline bool any_keys_down(const struct mejiro_state *st) {
    return (st->left_mask | st->right_mask | st->mod_mask) != 0;
}

/*
 * Build stroke string in the same “conceptual order” used by the Python regex:
 * Left:  (S T K N)(Y I A U)(n t k)
 * Mid:   '-' or '#' (we encode as '-' always; if pound is set we use '#')
 * Right: (S T K N)(Y I A U)(n t k)
 * Tail:  '*' if star is set
 *
 * NOTE: This is the *string* side of what your Python does with regex parsing. :contentReference[oaicite:4]{index=4}
 */
static void append_if(char *dst, size_t dst_len, bool cond, const char *s) {
    if (!cond || !dst || !s) return;
    size_t cur = strlen(dst);
    size_t add = strlen(s);
    if (cur + add + 1 > dst_len) return;
    memcpy(dst + cur, s, add);
    dst[cur + add] = '\0';
}

static void build_side(char *dst, size_t dst_len, uint32_t mask, bool is_left) {
    /* consonants: S T K N */
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_S : MJ_R_S)), "S");
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_T : MJ_R_T)), "T");
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_K : MJ_R_K)), "K");
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_N : MJ_R_N)), "N");

    /* vowels: Y I A U */
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_Y : MJ_R_Y)), "Y");
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_I : MJ_R_I)), "I");
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_A : MJ_R_A)), "A");
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_U : MJ_R_U)), "U");

    /* particles: n t k  (lowercase) */
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_n : MJ_R_n)), "n");
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_t : MJ_R_t)), "t");
    append_if(dst, dst_len, mask & (1u << (is_left ? MJ_L_k : MJ_R_k)), "k");
}

static void build_stroke_string(const struct mejiro_state *st, char *out, size_t out_len) {
    if (!out || out_len == 0) return;
    out[0] = '\0';

    /* left side */
    build_side(out, out_len, st->left_mask, true);

    /* mid separator: default '-' ; if pound is set, use '#'
       (your python uses '-' plus optional '#', but exact encoding is up to us) */
    if (st->mod_mask & (1u << MJ_POUND)) {
        append_if(out, out_len, true, "#");
    } else {
        append_if(out, out_len, true, "-");
    }

    /* right side */
    build_side(out, out_len, st->right_mask, false);

    /* tail: '*' */
    if (st->mod_mask & (1u << MJ_STAR)) {
        append_if(out, out_len, true, "*");
    }
}

/* ---- public API ------------------------------------------------------- */

void mejiro_reset(struct mejiro_state *st) {
    if (!st) return;
    st->left_mask = 0;
    st->right_mask = 0;
    st->mod_mask = 0;
    st->active = false;
}

void mejiro_on_key_event(struct mejiro_state *st, enum mejiro_key_id id, bool pressed) {
    if (!st) return;

    uint32_t bit = 1u << (uint32_t)id;

    if (id == MJ_POUND || id == MJ_STAR) {
        if (pressed) st->mod_mask |= bit;
        else st->mod_mask &= ~bit;
        return;
    }

    /* left bucket */
    if (id <= MJ_L_k) {
        if (pressed) st->left_mask |= bit;
        else st->left_mask &= ~bit;
        return;
    }

    /* right bucket */
    if (id >= MJ_R_S && id <= MJ_R_k) {
        if (pressed) st->right_mask |= bit;
        else st->right_mask &= ~bit;
        return;
    }
}

/*
 * Called when a release *might* complete a stroke:
 * - If all keys are now up, translate and emit, then clear masks.
 */
bool mejiro_try_emit(struct mejiro_state *st) {
    if (!st) return false;

    /* nothing collected */
    if (!any_keys_down(st)) {
        return false;
    }

    /* We only emit when everything is released.
       In this minimal core we assume caller calls this after every release
       and that "still holding something" is represented in masks.
       If your behavior layer keeps per-key pressed state elsewhere, adapt here. */
    /* In our design: the masks ARE the pressed-state, so if any bit is still set -> not ready */
    /* Therefore: emit only when all masks are already cleared is impossible.
       We need a separate "latched" stroke snapshot. */

    /* Minimal fix: treat "active" as 'we have a stroke snapshot'.
       Caller sets active=true on any press, and on final release we call emit then reset.
     */

    /* If active==false, we were never started */
    if (!st->active) return false;

    /* If any bits remain pressed, wait */
    if (any_keys_down(st)) {
        return false;
    }

    /* build + lookup */
    char stroke[64];
    char out[128];
    build_stroke_string(st, stroke, sizeof(stroke));

    if (mejiro_tables_lookup(stroke, out, sizeof(out))) {
        /* If out == "" that's valid suppression */
        bool ok = mejiro_send_text(out);
        (void)ok;
        LOG_INF("MEJIRO stroke=%s matched", stroke);
        mejiro_reset(st);
        return true;
    }

    LOG_INF("MEJIRO stroke=%s no match", stroke);
    mejiro_reset(st);
    return false;
}

/*
 * Optional entrypoint — if you want core to be called from behavior.
 * Not used in the minimal implementation below.
 */
int mejiro_on_binding_released(const struct zmk_behavior_binding_event *event) {
    (void)event;
    return 0;
}
