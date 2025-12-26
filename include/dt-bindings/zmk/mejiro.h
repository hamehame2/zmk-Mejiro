#pragma once
/*
 * Mejiro key IDs for &mj behavior param1
 *
 * 左右を完全に別IDにする。H(#) と X(*) は専用ID。
 *
 * Stroke表記は原典(mejiro_base.py)に合わせて：
 *  - 左:  STKNYIAUntk
 *  - 右:  STKNYIAUntk
 *  - '#' は中置（右がある場合は -#...）、右が無ければ末尾（tk# など）
 *  - '*' は末尾
 */

#define MJ_L_S   0
#define MJ_L_T   1
#define MJ_L_K   2
#define MJ_L_N   3
#define MJ_L_Y   4
#define MJ_L_I   5
#define MJ_L_A   6
#define MJ_L_U   7
#define MJ_L_n   8
#define MJ_L_t   9
#define MJ_L_k   10

#define MJ_H     11   /* # */

#define MJ_R_S   12
#define MJ_R_T   13
#define MJ_R_K   14
#define MJ_R_N   15
#define MJ_R_Y   16
#define MJ_R_I   17
#define MJ_R_A   18
#define MJ_R_U   19
#define MJ_R_n   20
#define MJ_R_t   21
#define MJ_R_k   22

#define MJ_X     23   /* * */

/* ---- User-friendly aliases (あなたの要望: &mj L_n) ---- */
#define L_S MJ_L_S
#define L_T MJ_L_T
#define L_K MJ_L_K
#define L_N MJ_L_N
#define L_Y MJ_L_Y
#define L_I MJ_L_I
#define L_A MJ_L_A
#define L_U MJ_L_U
#define L_n MJ_L_n
#define L_t MJ_L_t
#define L_k MJ_L_k

#define H   MJ_H

#define R_S MJ_R_S
#define R_T MJ_R_T
#define R_K MJ_R_K
#define R_N MJ_R_N
#define R_Y MJ_R_Y
#define R_I MJ_R_I
#define R_A MJ_R_A
#define R_U MJ_R_U
#define R_n MJ_R_n
#define R_t MJ_R_t
#define R_k MJ_R_k

#define X   MJ_X

/* ---- Backward-compat aliases (もし既存で MJ_L_n を使っていても壊さない) ---- */
#define MJ_L_n   MJ_L_n
#define MJ_L_t   MJ_L_t
#define MJ_L_k   MJ_L_k
#define MJ_R_n   MJ_R_n
#define MJ_R_t   MJ_R_t
#define MJ_R_k   MJ_R_k
