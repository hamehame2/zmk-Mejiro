/*
 * SPDX-License-Identifier: MIT
 *
 * Key-id definitions for Mejiro.
 */
#pragma once

#include <stdint.h>

/*
 * Bit packing rule used by core:
 * - Left  : 0..15
 * - Right : 16..31  (bit index = id - 16)
 * - Mod   : 32..47  (bit index = id - 32)
 *
 * You can expand later; for now keep minimum stable IDs.
 */
enum mejiro_key_id {
    /* Left keys (0..15) */
    MJ_L_0 = 0,
    MJ_L_1,
    MJ_L_2,
    MJ_L_3,
    MJ_L_4,
    MJ_L_5,
    MJ_L_6,
    MJ_L_7,
    MJ_L_8,
    MJ_L_9,
    MJ_L_10,
    MJ_L_11,
    MJ_L_12,
    MJ_L_13,
    MJ_L_14,
    MJ_L_15,

    /* Right keys (16..31) */
    MJ_R_0 = 16,
    MJ_R_1,
    MJ_R_2,
    MJ_R_3,
    MJ_R_4,
    MJ_R_5,
    MJ_R_6,
    MJ_R_7,
    MJ_R_8,
    MJ_R_9,
    MJ_R_10,
    MJ_R_11,
    MJ_R_12,
    MJ_R_13,
    MJ_R_14,
    MJ_R_15,

    /* Mods (32..) */
    MJ_MOD_H = 32, /* '#' */
    MJ_MOD_X = 33, /* '*' */
};
