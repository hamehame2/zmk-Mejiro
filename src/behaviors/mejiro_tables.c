#include "mejiro/mejiro_tables.h"
#include <string.h>

/*
 * IMPORTANT:
 * This is a minimal table lookup example.
 * Replace/extend with your real Mejiro dictionary.
 */

struct mj_entry {
    const char *stroke;
    const char *roman;
};

static const struct mj_entry mj_table[] = {
    /* examples */
    { "S",   "a" },
    { "T",   "i" },
    { "K",   "u" },
    { "N",   "e" },
    { "Y",   "o" },
    { "S-T", "ka" },
};

static bool mask_has(uint32_t mask, enum mejiro_key_id id) {
    return (mask & mj_bit(id)) != 0;
}

void mejiro_build_stroke_string(uint32_t left_mask, uint32_t right_mask, uint32_t mod_mask,
                               char *out, size_t out_len) {
    if (!out || out_len == 0) return;
    out[0] = '\0';

    /* left order: STKNYIAUntk */
    const struct { enum mejiro_key_id id; const char *ch; } L[] = {
        {MJ_L_S,"S"},{MJ_L_T,"T"},{MJ_L_K,"K"},{MJ_L_N,"N"},{MJ_L_Y,"Y"},
        {MJ_L_I,"I"},{MJ_L_A,"A"},{MJ_L_U,"U"},{MJ_L_n,"n"},{MJ_L_t,"t"},{MJ_L_k,"k"},
    };
    const struct { enum mejiro_key_id id; const char *ch; } R[] = {
        {MJ_R_S,"S"},{MJ_R_T,"T"},{MJ_R_K,"K"},{MJ_R_N,"N"},{MJ_R_Y,"Y"},
        {MJ_R_I,"I"},{MJ_R_A,"A"},{MJ_R_U,"U"},{MJ_R_n,"n"},{MJ_R_t,"t"},{MJ_R_k,"k"},
    };

    /* '#' and '*' */
    const bool has_hash = (mod_mask & mj_bit(MJ_POUND)) != 0;
    const bool has_star = (mod_mask & mj_bit(MJ_STAR))  != 0;

    /* append left */
    for (size_t i = 0; i < sizeof(L)/sizeof(L[0]); i++) {
        if (mask_has(left_mask, L[i].id)) strncat(out, L[i].ch, out_len - strlen(out) - 1);
    }
    if (has_hash) strncat(out, "#", out_len - strlen(out) - 1);

    /* append right (if any) with '-' */
    bool any_right = false;
    for (size_t i = 0; i < sizeof(R)/sizeof(R[0]); i++) {
        if (mask_has(right_mask, R[i].id)) { any_right = true; break; }
    }
    if (any_right) {
        strncat(out, "-", out_len - strlen(out) - 1);
        for (size_t i = 0; i < sizeof(R)/sizeof(R[0]); i++) {
            if (mask_has(right_mask, R[i].id)) strncat(out, R[i].ch, out_len - strlen(out) - 1);
        }
    }

    if (has_star) strncat(out, "*", out_len - strlen(out) - 1);
}

bool mejiro_lookup_roman(uint32_t left_mask, uint32_t right_mask, uint32_t mod_mask,
                        const char **out_roman) {
    static char stroke[64];
    mejiro_build_stroke_string(left_mask, right_mask, mod_mask, stroke, sizeof(stroke));

    for (size_t i = 0; i < sizeof(mj_table)/sizeof(mj_table[0]); i++) {
        if (strcmp(stroke, mj_table[i].stroke) == 0) {
            if (out_roman) *out_roman = mj_table[i].roman;
            return true;
        }
    }
    return false;
}
