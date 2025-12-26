#pragma once

/* ====== canonical numeric IDs (C内部の本体) ====== */
#define MJ_L_n  8
#define MJ_L_t  9
#define MJ_L_k 10

#define MJ_R_n 20
#define MJ_R_t 21
#define MJ_R_k 22

/* H (#), X (*) を左右で分けないなら canonical ID を1つだけ */
#define MJ_H   0
#define MJ_X   7

/* ====== user-facing aliases (DTSで使う短い名前) ====== */
#define L_n MJ_L_n
#define L_t MJ_L_t
#define L_k MJ_L_k

#define R_n MJ_R_n
#define R_t MJ_R_t
#define R_k MJ_R_k

#define H MJ_H
#define X MJ_X
