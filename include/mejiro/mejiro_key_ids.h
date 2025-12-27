/*
 * SPDX-License-Identifier: MIT
 *
 * Mejiro key id definitions
 * - behavior binding param1 = enum mejiro_key_id
 *
 * NOTE:
 * まず「動作確認用」に 0..31 を Left(0..15)/Right(16..31) に割り当てています。
 * ここはあなたの実キー配列に合わせて後で増減/並び替えOK。
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum mejiro_key_id {
    /* Left 0..15 */
    MJ_L0  = 0,
    MJ_L1  = 1,
    MJ_L2  = 2,
    MJ_L3  = 3,
    MJ_L4  = 4,
    MJ_L5  = 5,
    MJ_L6  = 6,
    MJ_L7  = 7,
    MJ_L8  = 8,
    MJ_L9  = 9,
    MJ_L10 = 10,
    MJ_L11 = 11,
    MJ_L12 = 12,
    MJ_L13 = 13,
    MJ_L14 = 14,
    MJ_L15 = 15,

    /* Right 16..31 */
    MJ_R0  = 16,
    MJ_R1  = 17,
    MJ_R2  = 18,
    MJ_R3  = 19,
    MJ_R4  = 20,
    MJ_R5  = 21,
    MJ_R6  = 22,
    MJ_R7  = 23,
    MJ_R8  = 24,
    MJ_R9  = 25,
    MJ_R10 = 26,
    MJ_R11 = 27,
    MJ_R12 = 28,
    MJ_R13 = 29,
    MJ_R14 = 30,
    MJ_R15 = 31,

    /* Optional modifier keys (not used in the minimal bring-up) */
    MJ_MOD0 = 32,
    MJ_MOD1 = 33,
    MJ_MOD2 = 34,
    MJ_MOD3 = 35,
    MJ_MOD4 = 36,
    MJ_MOD5 = 37,
    MJ_MOD6 = 38,
    MJ_MOD7 = 39,
};

#ifdef __cplusplus
}
#endif
