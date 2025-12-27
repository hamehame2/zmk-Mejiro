#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Mejiro key IDs (use as param1 in the &mj behavior) */
enum mejiro_key_id {
    /* Left side */
    MJ_L_S = 0,
    MJ_L_T,
    MJ_L_K,
    MJ_L_N,
    MJ_L_Y,
    MJ_L_I,
    MJ_L_A,
    MJ_L_U,

    /* Left particles */
    MJ_L_n,
    MJ_L_t,
    MJ_L_k,

    /* Right side */
    MJ_R_S,
    MJ_R_T,
    MJ_R_K,
    MJ_R_N,
    MJ_R_Y,
    MJ_R_I,
    MJ_R_A,
    MJ_R_U,

    /* Right particles */
    MJ_R_n,
    MJ_R_t,
    MJ_R_k,

    /* Mods */
    MJ_POUND, /* # */
    MJ_STAR,  /* * */

    MJ__COUNT
};

static inline uint32_t mejiro_bit(enum mejiro_key_id id) {
    return (uint32_t)1u << (uint32_t)id;
}

#ifdef __cplusplus
}
#endif
