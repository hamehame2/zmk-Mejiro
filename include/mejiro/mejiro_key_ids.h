/*
 * SPDX-License-Identifier: MIT
 *
 * Internal key-id definitions for Mejiro.
 * These IDs MUST match include/dt-bindings/zmk/mejiro.h
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum mejiro_key_id {
    /* Left (0..8) */
    MJ_L_S = 0,
    MJ_L_T = 1,
    MJ_L_K = 2,
    MJ_L_N = 3,
    MJ_L_n = 4,
    MJ_L_Y = 5,
    MJ_L_I = 6,
    MJ_L_A = 7,
    MJ_L_U = 8,

    /* Right (9..17) */
    MJ_R_S = 9,
    MJ_R_T = 10,
    MJ_R_K = 11,
    MJ_R_N = 12,
    MJ_R_n = 13,
    MJ_R_Y = 14,
    MJ_R_I = 15,
    MJ_R_A = 16,
    MJ_R_U = 17,

    /* Modifiers / specials */
    MJ_H = 18, /* # */
    MJ_X = 19, /* * */
};

#ifdef __cplusplus
}
#endif
