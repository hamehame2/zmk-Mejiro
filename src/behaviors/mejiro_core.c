// src/behaviors/mejiro_core.c
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <mejiro/mejiro_core.h>
#include <mejiro/mejiro_tables.h>
#include <mejiro/mejiro_key_ids.h>

LOG_MODULE_REGISTER(mejiro_core, CONFIG_ZMK_LOG_LEVEL);

/* -------------------------------------------------------------------------- */
/* State                                                                      */
/* -------------------------------------------------------------------------- */

static uint32_t held_mask;   /* keys currently held */
static uint32_t chord_mask;  /* keys that participated in this chord */

/* -------------------------------------------------------------------------- */
/* Table lookup helpers                                                        */
/* -------------------------------------------------------------------------- */

static const struct mj_kv *find_kv(const struct mj_kv *tbl, size_t len, const char *k) {
    for (size_t i = 0; i < len; i++) {
        if (tbl[i].k && !strcmp(tbl[i].k, k)) {
            return &tbl[i];
        }
    }
    return NULL;
}

/* -------------------------------------------------------------------------- */
/* Stroke normalization: [STKNYIAUntk#STKNYIAUntk*]                            */
/*   - Left letters in fixed order                                             */
/*   - Optional '#' appended after left letters                                */
/*   - If any right letters: add '-' then right letters in fixed order         */
/*   - Optional '*' appended at end                                            */
/* -------------------------------------------------------------------------- */

static const struct {
    uint8_t id;
    char ch;
} left_order[] = {
    {MJ_L_S, 'S'}, {MJ_L_T, 'T'}, {MJ_L_K, 'K'}, {MJ_L_N, 'N'},
    {MJ_L_Y, 'Y'}, {MJ_L_I, 'I'}, {MJ_L_A, 'A'}, {MJ_L_U, 'U'},
    {MJ_L_n, 'n'}, {MJ_L_t, 't'}, {MJ_L_k, 'k'},
};

static const struct {
    uint8_t id;
    char ch;
} right_order[] = {
    {MJ_R_S, 'S'}, {MJ_R_T, 'T'}, {MJ_R_K, 'K'}, {MJ_R_N, 'N'},
    {MJ_R_Y, 'Y'}, {MJ_R_I, 'I'}, {MJ_R_A, 'A'}, {MJ_R_U, 'U'},
    {MJ_R_n, 'n'}, {MJ_R_t, 't'}, {MJ_R_k, 'k'},
};

static bool build_stroke_from_mask(uint32_t mask, char *out, size_t out_sz) {
    if (!out || out_sz == 0) return false;
    out[0] = '\0';

    size_t w = 0;

    /* Left side */
    for (size_t i = 0; i < sizeof(left_order)/sizeof(left_order[0]); i++) {
        uint32_t bit = 1u << left_order[i].id;
        if (mask & bit) {
            if (w + 1 >= out_sz) return false;
            out[w++] = left_order[i].ch;
        }
    }

    /* '#' (H key) */
    if (mask & (1u << MJ_HASH)) {
        if (w + 1 >= out_sz) return false;
        out[w++] = '#';
    }

    /* Right side present? */
    bool any_right = false;
    for (size_t i = 0; i < sizeof(right_order)/sizeof(right_order[0]); i++) {
        if (mask & (1u << right_order[i].id)) {
            any_right = true;
            break;
        }
    }

    if (any_right) {
        if (w + 1 >= out_sz) return false;
        out[w++] = '-';

        for (size_t i = 0; i < sizeof(right_order)/sizeof(right_order[0]); i++) {
            uint32_t bit = 1u << right_order[i].id;
            if (mask & bit) {
                if (w + 1 >= out_sz) return false;
                out[w++] = right_order[i].ch;
            }
        }
    }

    /* '*' (X key) at end */
    if (mask & (1u << MJ_STAR)) {
        if (w + 1 >= out_sz) return false;
        out[w++] = '*';
    }

    out[w] = '\0';
    return (w > 0);
}

/* -------------------------------------------------------------------------- */
/* Commit                                                                      */
/* -------------------------------------------------------------------------- */

static void commit_if_ready(void) {
    if (held_mask != 0) return;     /* still holding */
    if (chord_mask == 0) return;

    char stroke[48];
    const bool ok = build_stroke_from_mask(chord_mask, stroke, sizeof(stroke));
    LOG_DBG("commit: mask=0x%08x stroke='%s'", chord_mask, ok ? stroke : "<none>");

    chord_mask = 0;
    if (!ok || stroke[0] == '\0') return;

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

/* -------------------------------------------------------------------------- */
/* Public API                                                                  */
/* -------------------------------------------------------------------------- */

void mejiro_core_on_press(uint8_t key_id) {
    if (key_id >= MJ_KEY_ID_MAX) return;

    uint32_t bit = 1u << key_id;
    held_mask |= bit;
    chord_mask |= bit;
}

void mejiro_core_on_release(uint8_t key_id) {
    if (key_id >= MJ_KEY_ID_MAX) return;

    uint32_t bit = 1u << key_id;
    held_mask &= ~bit;

    commit_if_ready();
}

void mejiro_core_reset(void) {
    held_mask = 0;
    chord_mask = 0;
}
