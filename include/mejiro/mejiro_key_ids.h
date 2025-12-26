// include/mejiro/mejiro_key_ids.h
#pragma once
#include <stdint.h>

/*
 * Key IDs passed via binding->param1 to &mj
 *
 * Layout concept (from your python comment):
 *   [STKNYIAUntk#STKNYIAUntk*]
 *
 * Stroke string normalization:
 *   LEFT:  STKNYIAUntk + optional '#'
 *   RIGHT: '-' + STKNYIAUntk (if any right key)
 *   STAR:  optional '*', appended at end
 */

enum mejiro_key_id {
    /* Left side (fixed order) */
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
    MJ_HASH,   /* '#' (your H key) */

    /* Right side (fixed order) */
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
