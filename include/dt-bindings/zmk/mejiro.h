#pragma once
/*
 * dt-bindings header for Mejiro (used by .keymap / devicetree overlay).
 * MUST contain only macros/constants usable by the C preprocessor for DTS.
 * Do NOT include normal C headers here.
 */

/* ---- Key ID (0..31) -------------------------------------------------- */
/*
 * 「0x01 をどこでどう処理する？」への答え：
 * ここでは “ID” を定義する。bitmask は C 側で (1u << id) に変換する。
 * これで将来拡張しても破綻しない（=後戻りしない）。
 */

/* Left hand */
#define MJ_L_S   0
#define MJ_L_T   1
#define MJ_L_K   2
#define MJ_L_N   3
#define MJ_L_Y   4
#define MJ_L_I   5
#define MJ_L_A   6
#define MJ_L_U   7

/* Right hand */
#define MJ_R_S   8
#define MJ_R_T   9
#define MJ_R_K   10
#define MJ_R_N   11
#define MJ_R_Y   12
#define MJ_R_I   13
#define MJ_R_A   14
#define MJ_R_U   15

/* Optional / thumb / special (必要になったら増やす) */
#define MJ_H     16   /* (#) */
#define MJ_X     17   /* (*) */

/* ---- Backward-compatible aliases ------------------------------------- */
/*
 * あなたが言ってる L_N / L_n 問題：
 * “後々コード合わせやすい” のは同意。
 * だから alias をここで両対応にして、出戻りをゼロにする。
 *
 * 例：MJ_L_N と MJ_L_n を両方定義しておけば、将来どっちに寄せても壊れない。
 */
#define MJ_L_n   MJ_L_N
#define MJ_R_n   MJ_R_N
