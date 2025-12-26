#pragma once
/*
 * Devicetree / keymap-facing constants for Mejiro.
 *
 * - This header is for DTS preprocessing (keymap .keymap).
 * - Do NOT include C headers here.
 * - Provide stable numeric IDs usable as: &mj L_n  (and also MJ_L_n).
 */

/* ---- Key IDs (must match the internal enum order) ----------------------
 * Source of truth: mejiro_key_ids.h enum (MJ_L_S..)
 * Keep these numbers stable once published.
 */

/* Left */
#define MJ_L_S   0
#define MJ_L_T   1
#define MJ_L_K   2
#define MJ_L_N   3
#define MJ_L_n   4
#define MJ_L_Y   5
#define MJ_L_I   6
#define MJ_L_A   7
#define MJ_L_U   8

/* Right */
#define MJ_R_S   9
#define MJ_R_T   10
#define MJ_R_K   11
#define MJ_R_N   12
#define MJ_R_n   13
#define MJ_R_Y   14
#define MJ_R_I   15
#define MJ_R_A   16
#define MJ_R_U   17

/* Modifiers / specials */
#define MJ_H     18
#define MJ_X     19

/* ---- Short aliases (requested): allow "&mj L_n" etc ------------------- */
/* Left aliases */
#define L_S   MJ_L_S
#define L_T   MJ_L_T
#define L_K   MJ_L_K
#define L_N   MJ_L_N
#define L_n   MJ_L_n
#define L_Y   MJ_L_Y
#define L_I   MJ_L_I
#define L_A   MJ_L_A
#define L_U   MJ_L_U

/* Right aliases */
#define R_S   MJ_R_S
#define R_T   MJ_R_T
#define R_K   MJ_R_K
#define R_N   MJ_R_N
#define R_n   MJ_R_n
#define R_Y   MJ_R_Y
#define R_I   MJ_R_I
#define R_A   MJ_R_A
#define R_U   MJ_R_U

/* Modifier aliases */
#define H     MJ_H
#define X     MJ_X
