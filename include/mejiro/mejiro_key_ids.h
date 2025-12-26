#pragma once
#include <stdint.h>

/*
 * Mejiro key id list
 *
 * Left / Right each have: S T K N Y I A U n t k
 * Extra keys: H (#), X (*)
 *
 * NOTE:
 * - enum 名は MJ_* を維持（内部用・衝突回避）
 * - keymap で使いやすい別名 L_* / R_* / H / X を macro で提供
 */

typedef enum {
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

    /* Specials */
    MJ_H, /* H (#) */
    MJ_X, /* X (*) */

    MJ_KEY_ID_MAX = MJ_X
} mj_key_id_t;

/* --- Aliases for keymap friendliness (requested) ----------------------- */
/* Left aliases */
#define L_S  MJ_L_S
#define L_T  MJ_L_T
#define L_K  MJ_L_K
#define L_N  MJ_L_N
#define L_Y  MJ_L_Y
#define L_I  MJ_L_I
#define L_A  MJ_L_A
#define L_U  MJ_L_U
#define L_n  MJ_L_n
#define L_t  MJ_L_t
#define L_k  MJ_L_k

/* Right aliases */
#define R_S  MJ_R_S
#define R_T  MJ_R_T
#define R_K  MJ_R_K
#define R_N  MJ_R_N
#define R_Y  MJ_R_Y
#define R_I  MJ_R_I
#define R_A  MJ_R_A
#define R_U  MJ_R_U
#define R_n  MJ_R_n
#define R_t  MJ_R_t
#define R_k  MJ_R_k

/* Special aliases */
#define H    MJ_H
#define X    MJ_X
