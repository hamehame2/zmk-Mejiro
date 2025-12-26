#pragma once
#include <stdint.h>

/*
 * Key IDs passed via binding->param1 to &mj
 *
 * Normalized stroke concept:
 *   Left  side:  S T K N Y I A U n t k + optional '#'
 *   Right side:  (if any right key) '-' + S T K N Y I A U n t k
 *   Optional '*': appended at the end
 *
 * NOTE:
 * - Enum order defines bit positions: (1u << id)
 * - Keep this stable to avoid "0x01 meaning changed" regressions.
 */

enum mejiro_key_id {
    /* Left hand */
    MJ_L_S = 0,
    MJ_L_T,
    MJ_L_K,
    MJ_L_N,
    MJ_L_Y,
    MJ_L_I,
    MJ_L_A,
    MJ_L_U,
    MJ_L_n,
    MJ_L_t,
    MJ_L_k,
    MJ_POUND,  /* '#' (your H key) */

    /* Right hand */
    MJ_R_S,
    MJ_R_T,
    MJ_R_K,
    MJ_R_N,
    MJ_R_Y,
    MJ_R_I,
    MJ_R_A,
    MJ_R_U,
    MJ_R_n,
    MJ_R_t,
    MJ_R_k,
    MJ_STAR,   /* '*' (your X key) */

    MJ_KEY_ID_MAX
};

/* Bit helpers (core uses these) */
static inline uint32_t mj_bit(enum mejiro_key_id id) {
    return (id < MJ_KEY_ID_MAX) ? (1u << (uint32_t)id) : 0u;
}
