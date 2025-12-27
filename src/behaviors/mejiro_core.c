/*
 * SPDX-License-Identifier: MIT
 *
 * Mejiro core implementation (minimal bring-up)
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/logging/log.h>
#include <mejiro/mejiro_core.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* from mejiro_send_roman.c */
bool mejiro_send_text(const char *text);

/* from mejiro_tables.c */
size_t mejiro_tables_build_output(uint32_t left_mask, uint32_t right_mask, uint32_t mod_mask,
                                  char *out, size_t out_cap);

void mejiro_reset(struct mejiro_state *s) {
    if (!s) return;
    s->active = false;
    s->left_mask = 0;
    s->right_mask = 0;
    s->mod_mask = 0;
}

void mejiro_on_key_event(struct mejiro_state *s, enum mejiro_key_id id, bool pressed) {
    if (!s) return;

    uint32_t bit = 0;
    if (id >= MJ_L0 && id <= MJ_L15) {
        bit = 1u << (uint32_t)(id - MJ_L0);
        if (pressed) s->left_mask |= bit;
        else s->left_mask &= ~bit;
        return;
    }

    if (id >= MJ_R0 && id <= MJ_R15) {
        bit = 1u << (uint32_t)(id - MJ_R0);
        if (pressed) s->right_mask |= bit;
        else s->right_mask &= ~bit;
        return;
    }

    if (id >= MJ_MOD0 && id <= MJ_MOD7) {
        bit = 1u << (uint32_t)(id - MJ_MOD0);
        if (pressed) s->mod_mask |= bit;
        else s->mod_mask &= ~bit;
        return;
    }

    /* unknown id: ignore */
}

bool mejiro_try_emit(const struct mejiro_state *stroke) {
    if (!stroke || !stroke->active) {
        return false;
    }

    char out[64];
    out[0] = '\0';

    (void)mejiro_tables_build_output(stroke->left_mask, stroke->right_mask, stroke->mod_mask,
                                    out, sizeof(out));

    LOG_INF("MEJIRO stroke (L=%08x R=%08x M=%08x) => \"%s\"",
            stroke->left_mask, stroke->right_mask, stroke->mod_mask, out);

    /* actually send */
    return mejiro_send_text(out);
}
