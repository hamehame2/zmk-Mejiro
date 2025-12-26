#pragma once
/*
 * dt-bindings for zmk,behavior-mejiro
 *
 * 24 keys total:
 *   Left  : S T K N Y I A U n t k H(#)  = 12
 *   Right : S T K N Y I A U n t k X(*)  = 12
 *
 * NOTE:
 * - Uppercase (e.g. L_N) and lowercase (e.g. L_n) are different IDs.
 * - H means '#', X means '*'. They are NOT left/right separated.
 *
 * ID assignment keeps your previous numbers:
 *   MJ_L_n=8, MJ_L_t=9, MJ_L_k=10
 *   MJ_R_n=20, MJ_R_t=21, MJ_R_k=22
 */

/* ---- Canonical numeric IDs (MUST be numbers for devicetree) ---- */
/* Left hand */
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

#define MJ_H     11   /* '#' */

/* Right hand */
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

#define MJ_X     23   /* '*' */

/* ---- Optional helper macro for bitmask usage in C (not for DTS) ---- */
#define MJ_BIT(id) (1u << (id))

/* ---- User-friendly aliases (your request: allow &mj L_n etc.) ---- */
/* Left uppercase */
#define L_S MJ_L_S
#define L_T MJ_L_T
#define L_K MJ_L_K
#define L_N MJ_L_N
#define L_Y MJ_L_Y
#define L_I MJ_L_I
#define L_A MJ_L_A
#define L_U MJ_L_U
/* Left lowercase */
#define L_n MJ_L_n
#define L_t MJ_L_t
#define L_k MJ_L_k

/* Right uppercase */
#define R_S MJ_R_S
#define R_T MJ_R_T
#define R_K MJ_R_K
#define R_N MJ_R_N
#define R_Y MJ_R_Y
#define R_I MJ_R_I
#define R_A MJ_R_A
#define R_U MJ_R_U
/* Right lowercase */
#define R_n MJ_R_n
#define R_t MJ_R_t
#define R_k MJ_R_k

/* Specials */
#define H   MJ_H
#define X   MJ_X

/*
 * Backward-compat (ONLY if you really need it):
 *  - DO NOT do self-referential defines like: #define MJ_L_n MJ_L_n
 *  - If you must support old spellings, map them to numbers or canonical names.
 *
 * Example (uncomment only if needed):
 *   #define MJ_L_SOME_OLD_NAME MJ_L_S
 */
