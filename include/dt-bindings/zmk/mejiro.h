/*
 * SPDX-License-Identifier: MIT
 *
 * dt-bindings for Mejiro behavior parameter
 * これが無いと keymap 内で &mj MJ_L0 の MJ_L0 が “数値”にならず dtc が落ちる
 */

#pragma once

/* 例: あなたの key_id が 0.. で並んでいる前提で最低限だけ用意 */
#define MJ_L0 0
#define MJ_L1 1
#define MJ_L2 2
#define MJ_L3 3
#define MJ_L4 4
#define MJ_L5 5
#define MJ_L6 6
#define MJ_L7 7

#define MJ_R0 8
#define MJ_R1 9
#define MJ_R2 10
#define MJ_R3 11
#define MJ_R4 12
#define MJ_R5 13
#define MJ_R6 14
#define MJ_R7 15

/* mod 系があるならここに追加 */
#define MJ_MOD0 16
#define MJ_MOD1 17
#define MJ_MOD2 18
#define MJ_MOD3 19
