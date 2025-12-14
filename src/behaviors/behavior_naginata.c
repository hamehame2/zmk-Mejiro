/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_naginata

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>

#include <zmk_naginata/nglist.h>
#include <zmk_naginata/nglistarray.h>

#include <zmk_naginata/naginata_func.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct naginata_config {
    bool tategaki;
    int os;
};

//struct naginata_config naginata_config;

static uint32_t pressed_keys = 0UL; // 押しているキーのビットをたてる
static int8_t n_pressed_keys = 0;   // 押しているキーの数
static int64_t timestamp;

/* ==============================
 *  Mejiro core (separate file)
 * ============================== */
extern bool mej_type_once(const NGList *keys);

/* Mejiro on/off.
 * Use F20 to enable, F21 to disable, F22 to toggle (bind from keymap).
 * When disabled, this behavior passes through key events as normal &kp.
 */
static bool mej_enabled = false;

static inline void mej_passthrough(uint32_t keycode, bool pressed, int64_t ts) {
    raise_zmk_keycode_state_changed_from_encoded(keycode, pressed, ts);
}
static NGListArray nginput;
extern bool ng_excluded;
extern bool ng_enabled;
extern uint32_t (*ng_mode_keymap)[2];

/*
 * Windows の全角とMac, iOS, Androidの全角
 *  0x0001-0x00FF はUSキーボードそのまま
 * Windows: ｧ 00C0 ｨ 00C1 ｩ 00C2 ｪ 00C3 ｫ 00C4 ｯ 00C5 ｬ 00C6 ｭ 00C7 ｮ 00C8
 *          ｡ 00C9 ｢ 00CA ｣ 00CB ､ 00CC ･ 00CD ｦ 00CE ｱ 00CF ｲ 00D0 ｳ 00D1 ｴ 00D2 ｵ 00D3
 *          ｶ 00D4 ｷ 00D5 ｸ 00D6 ｹ 00D7 ｺ 00D8 ｻ 00D9 ｼ 00DA ｽ 00DB ｾ 00DC ｿ 00DD ﾀ 00DE
 *          ﾁ 00DF ﾂ 00E0 ﾃ 00E1 ﾄ 00E2 ﾅ 00E3 ﾆ 00E4 ﾇ 00E5 ﾈ 00E6 ﾉ 00E7 ﾊ 00E8 ﾋ 00E9
 *          ﾌ 00EA ﾍ 00EB ﾎ 00EC ﾏ 00ED ﾐ 00EE ﾑ 00EF ﾒ 00F0 ﾓ 00F1 ﾔ 00F2 ﾕ 00F3 ﾖ 00F4
 *          ﾗ 00F5 ﾘ 00F6 ﾙ 00F7 ﾚ 00F8 ﾛ 00F9 ﾜ 00FA ﾝ 00FBﾞ 00FC ﾟ 00FD
 * Mac: ｧ 00A7 ｨ 00A8 ｩ 00A9 ｪ 00AA ｫ 00AB ｯ 00AC ｬ 00AD ｭ 00AE ｮ 00AF
 *      ｡ 00B0 ｢ 00B1 ｣ 00B2 ､ 00B3 ･ 00B4 ｦ 00B5 ｱ 00B6 ｲ 00B7 ｳ 00B8 ｴ 00B9 ｵ 00BA
 *      ｶ 00BB ｷ 00BC ｸ 00BD ｹ 00BE ｺ 00BF ｻ 00C0 ｼ 00C1 ｽ 00C2 ｾ 00C3 ｿ 00C4 ﾀ 00C5
 *      ﾁ 00C6 ﾂ 00C7 ﾃ 00C8 ﾄ 00C9 ﾅ 00CA ﾆ 00CB ﾇ 00CC ﾈ 00CD ﾉ 00CE ﾊ 00CF ﾋ 00D0
 *      ﾌ 00D1 ﾍ 00D2 ﾎ 00D3 ﾏ 00D4 ﾐ 00D5 ﾑ 00D6 ﾒ 00D7 ﾓ 00D8 ﾔ 00D9 ﾕ 00DA ﾖ 00DB
 *      ﾗ 00DC ﾘ 00DD ﾙ 00DE ﾚ 00DF ﾛ 00E0 ﾜ 00E1 ﾝ 00E2ﾞ 00E3 ﾟ 00E4
 * iOS, Android: ｧ 00A7 ｨ 00A8 ｩ 00A9 ｪ 00AA ｫ 00AB ｯ 00AC ｬ 00AD ｭ 00AE ｮ 00AF
 *               ｡ 00B0 ｢ 00B1 ｣ 00B2 ､ 00B3 ･ 00B4 ｦ 00B5 ｱ 00B6 ｲ 00B7 ｳ 00B8 ｴ 00B9 ｵ 00BA
 *               ｶ 00BB ｷ 00BC ｸ 00BD ｹ 00BE ｺ 00BF ｻ 00C0 ｼ 00C1 ｽ 00C2 ｾ 00C3 ｿ 00C4 ﾀ 00C5
 *               ﾁ 00C6 ﾂ 00C7 ﾃ 00C8 ﾄ 00C9 ﾅ 00CA ﾆ 00CB ﾇ 00CC ﾈ 00CD ﾉ 00CE ﾊ 00CF ﾋ 00D0
 *               ﾌ 00D1 ﾍ 00D2 ﾎ 00D3 ﾏ 00D4 ﾐ 00D5 ﾑ 00D6 ﾒ 00D7 ﾓ 00D8 ﾔ 00D9 ﾕ 00DA ﾖ 00DB
 *               ﾗ 00DC ﾘ 00DD ﾙ 00DE ﾚ 00DF ﾛ 00E0 ﾜ 00E1 ﾝ 00E2ﾞ 00E3 ﾟ 00E4
 */

/* shift = 00, douji = 01がシフトが効くキー */

#define NONE 0

// 薙刀式

// 31キーを32bitの各ビットに割り当てる
#define B_Q (1UL << 0)
#define B_W (1UL << 1)
#define B_E (1UL << 2)
#define B_R (1UL << 3)
#define B_T (1UL << 4)

#define B_Y (1UL << 5)
#define B_U (1UL << 6)
#define B_I (1UL << 7)
#define B_O (1UL << 8)
#define B_P (1UL << 9)

#define B_A (1UL << 10)
#define B_S (1UL << 11)
#define B_D (1UL << 12)
#define B_F (1UL << 13)
#define B_G (1UL << 14)

#define B_H (1UL << 15)
#define B_J (1UL << 16)
#define B_K (1UL << 17)
#define B_L (1UL << 18)
#define B_SEMI (1UL << 19)


#define B_Z (1UL << 20)
#define B_X (1UL << 21)
#define B_C (1UL << 22)
#define B_V (1UL << 23)
#define B_B (1UL << 24)

#define B_N (1UL << 25)
#define B_M (1UL << 26)
#define B_COMMA (1UL << 27)
#define B_DOT (1UL << 28)
#define B_SLASH (1UL << 29)

#define B_SPACE (1UL << 30)
#define B_SQT (1UL << 31)



#define NG_WINDOWS 0
#define NG_MACOS 1
#define NG_LINUX 2
#define NG_IOS 3

// EEPROMに保存する設定
typedef union {
    uint8_t os : 2;  // 2 bits can store values 0-3 (NG_WINDOWS, NG_MACOS, NG_LINUX, NG_IOS)
    bool tategaki : true; // true: 縦書き, false: 横書き
} user_config_t;

extern user_config_t naginata_config;

static const uint32_t ng_key[] = {
    [A - A] = B_A,     [B - A] = B_B,         [C - A] = B_C,         [D - A] = B_D,
    [E - A] = B_E,     [F - A] = B_F,         [G - A] = B_G,         [H - A] = B_H,
    [I - A] = B_I,     [J - A] = B_J,         [K - A] = B_K,         [L - A] = B_L,
    [M - A] = B_M,     [N - A] = B_N,         [O - A] = B_O,         [P - A] = B_P,
    [Q - A] = B_Q,     [R - A] = B_R,         [S - A] = B_S,         [T - A] = B_T,
    [U - A] = B_U,     [V - A] = B_V,         [W - A] = B_W,         [X - A] = B_X,
    [Y - A] = B_Y,     [Z - A] = B_Z,         [SEMI - A] = B_SEMI,   [COMMA - A] = B_COMMA, //[SQT - A] = B_SQT,
    //[COMMA - A] = B_COMMA, [DOT - A] = B_DOT, [SLASH - A] = B_SLASH, [SPACE - A] = B_SPACE,
    //[ENTER - A] = B_SPACE,
    [DOT - A] = B_DOT, [SLASH - A] = B_SLASH, [SPACE - A] = B_SPACE, [ENTER - A] = B_SPACE,
    [SQT - A] = B_SQT,
};

// カナ変換テーブル
typedef struct {
    uint32_t shift;
    uint32_t douji;
    uint32_t kana[6];
    void (*func)(void);
} naginata_kanamap;

static naginata_kanamap ngdickana[] = {
    // 清音/単打
    {.shift = NONE    , .douji = B_J            , .kana = {U, NONE, NONE, NONE, NONE, NONE}, .func = nofunc }, // う
    {.shift = NONE    , .douji = B_K            , .kana = {I, NONE, NONE, NONE, NONE, NONE}, .func = nofunc }, // い
    {.shift = NONE    , .douji = B_L            , .kana = {S, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // し

    {.shift = NONE    , .douji = B_F            , .kana = {N, N, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ん
    {.shift = NONE    , .douji = B_W            , .kana = {N, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // に
    {.shift = NONE    , .douji = B_H            , .kana = {K, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // く
    {.shift = NONE    , .douji = B_S            , .kana = {T, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // と
    //{.shift = NONE    , .douji = B_V            , .kana = {R, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // る////

    {.shift = NONE    , .douji = B_R            , .kana = {COMMA, SPACE, NONE, NONE, NONE, NONE}, .func = nofunc }, // 、変換
    {.shift = NONE    , .douji = B_O            , .kana = {G, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // が

    //{.shift = NONE    , .douji = B_B            , .kana = {T, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // つ////
    //{.shift = NONE    , .douji = B_N            , .kana = {T, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // て////

    {.shift = NONE    , .douji = B_E            , .kana = {H, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // は
    {.shift = NONE    , .douji = B_D            , .kana = {K, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // か
    //{.shift = NONE    , .douji = B_M            , .kana = {T, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // た///

    //{.shift = NONE    , .douji = B_C            , .kana = {K, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // き///
    {.shift = NONE    , .douji = B_X            , .kana = {M, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ま

    {.shift = NONE    , .douji = B_P            , .kana = {H, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ひ
    {.shift = NONE    , .douji = B_Z            , .kana = {S, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // す

    {.shift = NONE    , .douji = B_DOT          , .kana = {DOT, SPACE, NONE, NONE, NONE, NONE   }, .func = nofunc }, // 。変換

    {.shift = NONE    , .douji = B_I            , .kana = {K, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // こ
    {.shift = NONE    , .douji = B_SLASH        , .kana = {B, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぶ

    {.shift = NONE    , .douji = B_A            , .kana = {N, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // の

    //{.shift = NONE    , .douji = B_COMMA        , .kana = {D, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // で////
    {.shift = NONE    , .douji = B_SEMI         , .kana = {N, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // な
    {.shift = NONE    , .douji = B_Q         , .kana = {MINUS, NONE, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ー
    {.shift = NONE    , .douji = B_T         , .kana = {T, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ち
    {.shift = NONE    , .douji = B_G         , .kana = {X, T, U, NONE, NONE, NONE   }, .func = nofunc }, // っ
    {.shift = NONE    , .douji = B_Y         , .kana = {G, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぐ
    {.shift = NONE    , .douji = B_U         , .kana = {B, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ば
    // げ追加げはシフトに移行。NG SQTでもいけるようになりましたのでお好みで。
    {.shift = NONE    , .douji = B_SQT         , .kana = {G, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // げ


    
    //中指シフト
    {.shift = NONE    , .douji = B_K|B_Q        , .kana = {F, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ふぁ
    {.shift = NONE    , .douji = B_K|B_W        , .kana = {G, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ご
    {.shift = NONE     , .douji = B_K|B_E        , .kana = {H, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ふ
    {.shift = NONE    , .douji = B_K|B_R        , .kana = {F, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ふぃ
    {.shift = NONE    , .douji = B_K|B_T        , .kana = {F, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ふぇ
    {.shift = NONE     , .douji = B_D|B_Y        , .kana = {W, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // うぃ
    {.shift = NONE    , .douji = B_D|B_U        , .kana = {P, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぱ
    {.shift = NONE     , .douji = B_D|B_I        , .kana = {Y, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // よ
    {.shift = NONE    , .douji = B_D|B_O        , .kana = {M, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // み
    {.shift = NONE    , .douji = B_D|B_P        , .kana = {W, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // うぇ
    {.shift = NONE    , .douji = B_D|B_SQT     , .kana = {W, H, O, NONE, NONE, NONE      }, .func = nofunc }, // うぉ
    {.shift = NONE    , .douji = B_K|B_A        , .kana = {H, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ほ
    {.shift = NONE     , .douji = B_K|B_S        , .kana = {J, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // じ
    {.shift = NONE    , .douji = B_K|B_D        , .kana = {R, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // れ
    {.shift = NONE    , .douji = B_K|B_F        , .kana = {M, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // も
    {.shift = NONE    , .douji = B_K|B_G        , .kana = {Y, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ゆ
    {.shift = NONE    , .douji = B_D|B_H        , .kana = {H, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // へ
    {.shift = NONE     , .douji = B_D|B_J     , .kana = {A, NONE, NONE, NONE, NONE, NONE   }, .func = nofunc }, // あ

    {.shift = NONE     , .douji = B_D|B_SEMI        , .kana = {E, NONE, NONE, NONE, NONE, NONE   }, .func = nofunc }, // え
    {.shift = NONE     , .douji = B_K|B_Z        , .kana = {D, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // づ
    {.shift = NONE    , .douji = B_K|B_X        , .kana = {Z, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぞ
    {.shift = NONE    , .douji = B_K|B_C        , .kana = {B, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぼ
    {.shift = NONE    , .douji = B_K|B_V        , .kana = {M, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // む
    {.shift = NONE    , .douji = B_K|B_B        , .kana = {F, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ふぉ
    {.shift = NONE    , .douji = B_D|B_N        , .kana = {S, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // せ
    {.shift = NONE    , .douji = B_D|B_M        , .kana = {N, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ね
    {.shift = NONE    , .douji = B_D|B_COMMA        , .kana = {B, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // べ
    {.shift = NONE    , .douji = B_D|B_DOT        , .kana = {P, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぷ
    {.shift = NONE    , .douji = B_D|B_SLASH        , .kana = {V, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ゔ




    // 薬指シフト
    {.shift = NONE    , .douji = B_L|B_Q        , .kana = {D, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぢ
    {.shift = NONE    , .douji = B_L|B_W        , .kana = {M, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // め
    {.shift = NONE    , .douji = B_L|B_E        , .kana = {K, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // け
    {.shift = NONE    , .douji = B_L|B_R        , .kana = {T, H, I, NONE, NONE, NONE   }, .func = nofunc }, // てぃ
    {.shift = NONE    , .douji = B_L|B_T        , .kana = {D, H, I, NONE, NONE, NONE   }, .func = nofunc }, // でぃ
    {.shift = NONE    , .douji = B_S|B_Y        , .kana = {S, Y, E, NONE, NONE, NONE   }, .func = nofunc }, // しぇ
    {.shift = NONE    , .douji = B_S|B_U        , .kana = {P, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぺ
    {.shift = NONE    , .douji = B_S|B_I        , .kana = {D, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ど
    {.shift = NONE    , .douji = B_S|B_O        , .kana = {Y, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // や
    {.shift = NONE    , .douji = B_S|B_P        , .kana = {J, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // じぇ

    {.shift = NONE    , .douji = B_L|B_A        , .kana = {W, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // を
    {.shift = NONE    , .douji = B_L|B_S        , .kana = {S, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // さ
    {.shift = NONE    , .douji = B_L|B_D        , .kana = {O, NONE, NONE, NONE, NONE, NONE   }, .func = nofunc }, // お
    {.shift = NONE    , .douji = B_L|B_F        , .kana = {R, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // り
    {.shift = NONE    , .douji = B_L|B_G        , .kana = {Z, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ず
    {.shift = NONE    , .douji = B_S|B_H        , .kana = {B, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // び
    {.shift = NONE    , .douji = B_S|B_J        , .kana = {R, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ら

    {.shift = NONE    , .douji = B_S|B_SEMI        , .kana = {S, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // そ
    {.shift = NONE    , .douji = B_L|B_Z        , .kana = {Z, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぜ
    {.shift = NONE    , .douji = B_L|B_X        , .kana = {Z, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ざ
    {.shift = NONE    , .douji = B_L|B_C        , .kana = {G, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぎ
    {.shift = NONE    , .douji = B_L|B_V        , .kana = {R, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ろ
    {.shift = NONE    , .douji = B_L|B_B        , .kana = {N, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぬ
    {.shift = NONE    , .douji = B_S|B_N        , .kana = {W, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // わ
    {.shift = NONE    , .douji = B_S|B_M        , .kana = {D, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // だ
    {.shift = NONE    , .douji = B_S|B_COMMA        , .kana = {P, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぴ
    {.shift = NONE    , .douji = B_S|B_DOT        , .kana = {P, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぽ
    {.shift = NONE    , .douji = B_S|B_SLASH        , .kana = {T, Y, E, NONE, NONE, NONE   }, .func = nofunc }, // ちぇ


    // Iシフト
    {.shift = NONE    , .douji = B_I|B_Q        , .kana = {H, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // ひゅ
    {.shift = NONE    , .douji = B_I|B_W        , .kana = {S, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // しゅ
    {.shift = NONE    , .douji = B_I|B_E        , .kana = {S, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // しょ
    {.shift = NONE    , .douji = B_I|B_R        , .kana = {K, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // きゅ
    {.shift = NONE    , .douji = B_I|B_T        , .kana = {T, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // ちゅ
    {.shift = NONE    , .douji = B_I|B_A        , .kana = {H, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // ひょ
    {.shift = NONE    , .douji = B_I|B_F        , .kana = {K, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // きょ
    {.shift = NONE    , .douji = B_I|B_G        , .kana = {T, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // ちょ

    {.shift = NONE    , .douji = B_I|B_Z        , .kana = {H, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // ひゃ

    {.shift = NONE    , .douji = B_I|B_C        , .kana = {S, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // しゃ
    {.shift = NONE    , .douji = B_I|B_V        , .kana = {K, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // きゃ
    {.shift = NONE    , .douji = B_I|B_B        , .kana = {T, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // ちゃ

    {.shift = NONE    , .douji = B_I|B_X        , .kana = {MINUS, NONE, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ー追加
    // Oシフト
    {.shift = NONE    , .douji = B_O|B_Q        , .kana = {R, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // りゅ
    {.shift = NONE    , .douji = B_O|B_W        , .kana = {J, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // じゅ
    {.shift = NONE    , .douji = B_O|B_E        , .kana = {J, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // じょ
    {.shift = NONE    , .douji = B_O|B_R        , .kana = {G, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // ぎゅ
    {.shift = NONE    , .douji = B_O|B_T        , .kana = {N, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // にゅ
    {.shift = NONE    , .douji = B_O|B_A        , .kana = {R, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // りょ
    {.shift = NONE    , .douji = B_O|B_F        , .kana = {G, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // ぎょ
    {.shift = NONE    , .douji = B_O|B_G        , .kana = {N, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // にょ

    {.shift = NONE    , .douji = B_O|B_Z        , .kana = {R, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // りゃ

    {.shift = NONE    , .douji = B_O|B_C        , .kana = {Z, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // じゃ
    {.shift = NONE    , .douji = B_O|B_V        , .kana = {G, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // ぎゃ
    {.shift = NONE    , .douji = B_O|B_B        , .kana = {N, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // にゃ

    {.shift = NONE    , .douji = B_O|B_X        , .kana = {MINUS, NONE, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ー追加
    // IO3キー同時押しシフト
    {.shift = NONE    , .douji = B_I|B_O|B_Q        , .kana = {P, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // ぴゅ
    {.shift = NONE    , .douji = B_I|B_O|B_W        , .kana = {M, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // みゅ
    {.shift = NONE    , .douji = B_I|B_O|B_E        , .kana = {M, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // みょ
    {.shift = NONE    , .douji = B_I|B_O|B_R        , .kana = {B, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // びゅ
    {.shift = NONE    , .douji = B_I|B_O|B_T        , .kana = {D, H, U, NONE, NONE, NONE   }, .func = nofunc }, // でゅ
    {.shift = NONE    , .douji = B_I|B_O|B_A        , .kana = {P, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // ぴょ
    {.shift = NONE    , .douji = B_I|B_O|B_F        , .kana = {B, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // びょ
    {.shift = NONE    , .douji = B_I|B_O|B_G        , .kana = {V, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ゔぁ

    {.shift = NONE    , .douji = B_I|B_O|B_Z        , .kana = {P, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // ぴゃ

    {.shift = NONE    , .douji = B_I|B_O|B_C        , .kana = {M, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // みゃ
    {.shift = NONE    , .douji = B_I|B_O|B_V        , .kana = {B, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // びゃ
    {.shift = NONE    , .douji = B_I|B_O|B_B        , .kana = {V, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ゔぃ


    {.shift = NONE    , .douji = B_I|B_O|B_X        , .kana = {LS(INT1), NONE, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ＿追加
    // SQTシフト B_SQT 
    {.shift = NONE    , .douji = B_SQT|B_Q        , .kana = {S, U, X, I, NONE, NONE   }, .func = nofunc }, // スィ
    {.shift = NONE    , .douji = B_SQT|B_W        , .kana = {H, Y, E, NONE, NONE, NONE   }, .func = nofunc }, // ひぇ
    {.shift = NONE    , .douji = B_SQT|B_E        , .kana = {G, Y, E, NONE, NONE, NONE   }, .func = nofunc }, // ぎぇ
    {.shift = NONE    , .douji = B_SQT|B_R        , .kana = {P, Y, E, NONE, NONE, NONE   }, .func = nofunc }, // ぴぇ
    {.shift = NONE    , .douji = B_SQT|B_T        , .kana = {T, H, A, NONE, NONE, NONE   }, .func = nofunc }, // てゃ
    {.shift = NONE    , .douji = B_SQT|B_A        , .kana = {G, W, A, NONE, NONE, NONE   }, .func = nofunc }, // グァ
    {.shift = NONE    , .douji = B_SQT|B_F        , .kana = {BSPC, BSPC, BSPC, NONE, NONE, NONE   }, .func = nofunc }, // BS3
    {.shift = NONE    , .douji = B_SQT|B_G        , .kana = {G, W, O, NONE, NONE, NONE   }, .func = nofunc }, // ぐぉ

    {.shift = NONE    , .douji = B_SQT|B_Z        , .kana = {G, U, X, W, A, NONE   }, .func = nofunc }, // ぐゎ

    {.shift = NONE    , .douji = B_SQT|B_X        , .kana = {G, E, X, E, NONE, NONE   }, .func = nofunc }, // げぇ
    {.shift = NONE    , .douji = B_SQT|B_C        , .kana = {G, W, E, NONE, NONE, NONE   }, .func = nofunc }, // ぐぇ
    {.shift = NONE    , .douji = B_SQT|B_V        , .kana = {T, H, E, NONE, NONE, NONE   }, .func = nofunc }, // てぇ
    {.shift = NONE    , .douji = B_SQT|B_B        , .kana = {D, H, E, NONE, NONE, NONE   }, .func = nofunc }, // でぇ

    {.shift = NONE    , .douji = B_SQT|B_S        , .kana = {K, W, A, NONE, NONE, NONE   }, .func = nofunc }, // クァ



    // げうぉ外来音とネットスラングを親指のNGスペースにて
    {.shift = B_SPACE , .douji = B_Q            , .kana = {X, W, A, NONE, NONE, NONE   }, .func = nofunc }, // ゎ
    {.shift = B_SPACE , .douji = B_W            , .kana = {X, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // ゅ
    {.shift = B_SPACE , .douji = B_E            , .kana = {X, Y, O, NONE, NONE, NONE   }, .func = nofunc }, // ょ
    {.shift = B_SPACE , .douji = B_R            , .kana = {V, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // ヴュ
    {.shift = B_SPACE , .douji = B_T            , .kana = {T, H, U, NONE, NONE, NONE   }, .func = nofunc }, // てゅ
    {.shift = B_SPACE , .douji = B_Y            , .kana = {T, S, E, NONE, NONE, NONE   }, .func = nofunc }, // ツェ
    {.shift = B_SPACE , .douji = B_U            , .kana = {V, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ヴェ
    {.shift = B_SPACE , .douji = B_I            , .kana = {Q, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // くぇ
    {.shift = B_SPACE , .douji = B_O            , .kana = {X, K, E, NONE, NONE, NONE   }, .func = nofunc }, // ヶ
    {.shift = B_SPACE , .douji = B_P            , .kana = {X, K, A, NONE, NONE, NONE   }, .func = nofunc }, // ヵ
    {.shift = B_SPACE , .douji = B_A            , .kana = {X, A, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぁ
    {.shift = B_SPACE , .douji = B_S           , .kana = {X, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぃ
    {.shift = B_SPACE , .douji = B_D           , .kana = {X, U, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぅ
    {.shift = B_SPACE , .douji = B_F           , .kana = {X, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぇ
    {.shift = B_SPACE , .douji = B_G           , .kana = {X, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ぉ
    {.shift = B_SPACE , .douji = B_H           , .kana = {T, S, O, NONE, NONE, NONE   }, .func = nofunc }, // つぉ
    {.shift = B_SPACE , .douji = B_J           , .kana = {W, H, O, NONE, NONE, NONE   }, .func = nofunc }, // うぉ
    {.shift = B_SPACE , .douji = B_K           , .kana = {Q, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // くぉ
    {.shift = B_SPACE , .douji = B_L           , .kana = {G, E, NONE, NONE, NONE, NONE   }, .func = nofunc }, // げ
    {.shift = B_SPACE , .douji = B_SEMI           , .kana = {MINUS, NONE, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ー
    {.shift = B_SPACE , .douji = B_SQT          , .kana = {G, E, MINUS, NONE, NONE, NONE   }, .func = nofunc }, // げー
    {.shift = B_SPACE , .douji = B_Z           , .kana = {K, U, X, W, A, NONE   }, .func = nofunc }, // くゎ
    {.shift = B_SPACE , .douji = B_X           , .kana = {Y, E, MINUS, NONE, NONE, NONE   }, .func = nofunc }, // いぇー
    {.shift = B_SPACE , .douji = B_C           , .kana = {X, Y, A, NONE, NONE, NONE   }, .func = nofunc }, // ゃ
    {.shift = B_SPACE , .douji = B_V           , .kana = {F, Y, U, NONE, NONE, NONE   }, .func = nofunc }, // フュ
    {.shift = B_SPACE , .douji = B_B           , .kana = {V, O, NONE, NONE, NONE, NONE   }, .func = nofunc }, // ゔぉ
    {.shift = B_SPACE , .douji = B_N           , .kana = {T, S, I, NONE, NONE, NONE   }, .func = nofunc }, // つぃ
    {.shift = B_SPACE , .douji = B_M           , .kana = {T, S, A, NONE, NONE, NONE   }, .func = nofunc }, // つぁ
    {.shift = B_SPACE , .douji = B_COMMA           , .kana = {Q, I, NONE, NONE, NONE, NONE   }, .func = nofunc }, // くぃ
    {.shift = B_SPACE , .douji = B_DOT           , .kana = {T, W, U, NONE, NONE, NONE   }, .func = nofunc }, // とぅ
    {.shift = B_SPACE , .douji = B_SLASH           , .kana = {D, W, U, NONE, NONE, NONE   }, .func = nofunc }, // ドゥ

    // 追加
    {.shift = NONE    , .douji = B_SPACE        , .kana = {SPACE, NONE, NONE, NONE, NONE, NONE  }, .func = nofunc},
    //{.shift = B_SPACE , .douji = B_V            , .kana = {COMMA, ENTER, NONE, NONE, NONE, NONE }, .func = nofunc},
    //{.shift = NONE    , .douji = B_Q            , .kana = {NONE, NONE, NONE, NONE, NONE, NONE   }, .func = nofunc},
    //{.shift = B_SPACE , .douji = B_M            , .kana = {DOT, ENTER, NONE, NONE, NONE, NONE   }, .func = nofunc},
    //{.shift = NONE    , .douji = B_U            , .kana = {BSPC, NONE, NONE, NONE, NONE, NONE   }, .func = nofunc},

    //{.shift = NONE    , .douji = B_V|B_M        , .kana = {ENTER, NONE, NONE, NONE, NONE, NONE  }, .func = nofunc}, // enter////
    // {.shift = B_SPACE, .douji = B_V|B_M, .kana = {ENTER, NONE, NONE, NONE, NONE, NONE}, .func = nofunc}, // enter+シフト(連続シフト)

    //{.shift = NONE    , .douji = B_T            , .kana = {NONE, NONE, NONE, NONE, NONE, NONE   }, .func = ng_T}, //
    //{.shift = NONE    , .douji = B_Y            , .kana = {NONE, NONE, NONE, NONE, NONE, NONE   }, .func = ng_Y}, //
    //{.shift = B_SPACE , .douji = B_T            , .kana = {NONE, NONE, NONE, NONE, NONE, NONE   }, .func = ng_ST}, //
    //{.shift = B_SPACE , .douji = B_Y            , .kana = {NONE, NONE, NONE, NONE, NONE, NONE   }, .func = ng_SY}, //

//{.shift = NONE    , .douji = B_H|B_J        , .kana = {NONE, NONE, NONE, NONE, NONE, NONE   }, .func = naginata_on}, // 　かなオン///
    // {.shift = NONE, .douji = B_F | B_G, .kana = {NONE, NONE, NONE, NONE, NONE, NONE}, .func = naginata_off}, // 　かなオフ

    // 編集モード
    {.shift = B_J|B_K    , .douji = B_Q     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKQ    }, // ^{End}
    {.shift = B_J|B_K    , .douji = B_W     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKW    }, // ／{改行}
    {.shift = B_J|B_K    , .douji = B_E     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKE    }, // /*ディ*/
    {.shift = B_J|B_K    , .douji = B_R     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKR    }, // ^s
    {.shift = B_J|B_K    , .douji = B_T     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKT    }, // ・
    {.shift = B_J|B_K    , .douji = B_A     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKA    }, // ……{改行}
    //{.shift = B_J|B_K    , .douji = B_A     , .kana = {RBKT, BSLH, ENTER, LEFT, NONE, NONE} , .func = nofunc    }, // 「」
    {.shift = B_J|B_K    , .douji = B_S     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKS    }, // 『{改行}
    //{.shift = B_J|B_K    , .douji = B_S     , .kana = {LS(N8), LS(N9), ENTER, LEFT, NONE, NONE} , .func = nofunc    }, // （）
    {.shift = B_J|B_K    , .douji = B_D     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKD    }, // ？{改行}
    {.shift = B_J|B_K    , .douji = B_F     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKF    }, // 「{改行}
    {.shift = B_J|B_K    , .douji = B_G     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKG    }, // ({改行}
    //{.shift = B_J|B_K    , .douji = B_Z     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKZ    }, // ――{改行}
    {.shift = B_J|B_K    , .douji = B_Z     , .kana = {LS(N5), NONE, NONE, NONE, NONE, NONE} , .func = nofunc    }, // %
    //{.shift = B_J|B_K    , .douji = B_X     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKX    }, // 』{改行}
    {.shift = B_J|B_K    , .douji = B_X     , .kana = {LS(EQUAL), NONE, NONE, NONE, NONE, NONE} , .func = nofunc    }, // ～
    {.shift = B_J|B_K    , .douji = B_C     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKC    }, // ！{改行}
    {.shift = B_J|B_K    , .douji = B_V     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKV    }, // 」{改行}
    {.shift = B_J|B_K    , .douji = B_B     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_JKB    }, // ){改行}
    {.shift = B_D|B_F    , .douji = B_Y     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFY    }, // {Home}
    {.shift = B_D|B_F    , .douji = B_U     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFU    }, // +{End}{BS}
    {.shift = B_D|B_F    , .douji = B_I     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFI    }, // {vk1Csc079}
    //{.shift = B_D|B_F    , .douji = B_I     , .kana = {LWIN, SLASH, NONE, NONE, NONE, NONE} , .func = nofunc    }, // {vk1Csc079}
    {.shift = B_D|B_F    , .douji = B_O     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFO    }, // {Del}
    {.shift = B_D|B_F    , .douji = B_P     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFP    }, // +{Esc 2}
    {.shift = B_D|B_F    , .douji = B_H     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFH    }, // {Enter}{End}
    {.shift = B_D|B_F    , .douji = B_J     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFJ    }, // {↑}
    {.shift = B_D|B_F    , .douji = B_K     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFK    }, // +{↑}
    {.shift = B_D|B_F    , .douji = B_L     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFL    }, // +{↑ 7}
    //{.shift = B_D|B_F    , .douji = B_SEMI  , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFSCLN }, // ^i
    {.shift = B_D|B_F    , .douji = B_SEMI  , .kana = {F7, NONE, NONE, NONE, NONE, NONE} , .func = nofunc }, // ^i
    {.shift = B_D|B_F    , .douji = B_N     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFN    }, // {End}
    {.shift = B_D|B_F    , .douji = B_M     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFM    }, // {↓}
    {.shift = B_D|B_F    , .douji = B_COMMA , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFCOMM }, // +{↓}
    {.shift = B_D|B_F    , .douji = B_DOT   , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFDOT  }, // +{↓ 7}
    //{.shift = B_D|B_F    , .douji = B_SLASH , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_DFSLSH }, // ^u
    {.shift = B_D|B_F    , .douji = B_SLASH  , .kana = {F6, NONE, NONE, NONE, NONE, NONE} , .func = nofunc }, // ^u
    {.shift = B_M|B_COMMA, .douji = B_Q     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCQ    }, // ｜{改行}
    {.shift = B_M|B_COMMA, .douji = B_W     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCW    }, // 　　　×　　　×　　　×{改行 2}
    {.shift = B_M|B_COMMA, .douji = B_E     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCE    }, // {Home}{→}{End}{Del 2}{←}
    {.shift = B_M|B_COMMA, .douji = B_R     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCR    }, // {Home}{改行}{Space 1}{←}
    {.shift = B_M|B_COMMA, .douji = B_T     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCT    }, // 〇{改行}
    {.shift = B_M|B_COMMA, .douji = B_A     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCA    }, // 《{改行}
    {.shift = B_M|B_COMMA, .douji = B_S     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCS    }, // 【{改行}
    {.shift = B_M|B_COMMA, .douji = B_D     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCD    }, // {Home}{→}{End}{Del 4}{←}
    {.shift = B_M|B_COMMA, .douji = B_F     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCF    }, // {Home}{改行}{Space 3}{←}
    {.shift = B_M|B_COMMA, .douji = B_G     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCG    }, // {Space 3}
    {.shift = B_M|B_COMMA, .douji = B_Z     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCZ    }, // 》{改行}
    {.shift = B_M|B_COMMA, .douji = B_X     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCX    }, // 】{改行}
    {.shift = B_M|B_COMMA, .douji = B_C     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCC    }, // 」{改行}{改行}
    {.shift = B_M|B_COMMA, .douji = B_V     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCV    }, // 」{改行}{改行}「{改行}
    {.shift = B_M|B_COMMA, .douji = B_B     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_MCB    }, // 」{改行}{改行}{Space}
    {.shift = B_C|B_V    , .douji = B_Y     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVY    }, // +{Home}
    {.shift = B_C|B_V    , .douji = B_U     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVU    }, // ^x
    {.shift = B_C|B_V    , .douji = B_I     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVI    }, // {vk1Csc079}
    {.shift = B_C|B_V    , .douji = B_O     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVO    }, // ^v
    {.shift = B_C|B_V    , .douji = B_P     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVP    }, // ^z
    {.shift = B_C|B_V    , .douji = B_H     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVH    }, // ^c
    {.shift = B_C|B_V    , .douji = B_J     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVJ    }, // {←}
    {.shift = B_C|B_V    , .douji = B_K     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVK    }, // {→}
    {.shift = B_C|B_V    , .douji = B_L     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVL    }, // {改行}{Space}+{Home}^x{BS}
    {.shift = B_C|B_V    , .douji = B_SEMI  , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVSCLN }, // ^y
    {.shift = B_C|B_V    , .douji = B_N     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVN    }, // +{End}
    {.shift = B_C|B_V    , .douji = B_M     , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVM    }, // +{←}
    {.shift = B_C|B_V    , .douji = B_COMMA , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVCOMM }, // +{→}
    {.shift = B_C|B_V    , .douji = B_DOT   , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVDOT  }, // +{← 7}
    {.shift = B_C|B_V    , .douji = B_SLASH , .kana = {NONE, NONE, NONE, NONE, NONE, NONE} , .func = ngh_CVSLSH }, // +{→ 7}

};





/* ============================================================
 * ここからメジロ対応のための「左右分離」追加コード
 * ============================================================ */

// QWERTY 左手 / 右手判別
static inline bool ng_is_left_key(uint32_t keycode) {
    switch (keycode) {
        // 左手
        case Q: case W: case E: case R: case T:
        case A: case S: case D: case F: case G:
        case Z: case X: case C: case V: case B:
            return true;

        // 右手（＆その他）は false
        case Y: case U: case I: case O: case P:
        case H: case J: case K: case L: case SEMI: case SQT:
        case N: case M: case COMMA: case DOT: case SLASH:
        case SPACE: case ENTER:
        default:
            return false;
    }
}

// NGList を「左→右」の順に並べ替える（stable）
static void ng_stable_left_first(const NGList *in, NGList *out) {
    initializeList(out);

    // まず左手キーを順番通りに追加
    for (int i = 0; i < in->size; i++) {
        uint32_t kc = in->elements[i];
        if (ng_is_left_key(kc)) {
            addToList(out, kc);
        }
    }

    // 次に右手キーを順番通りに追加
    for (int i = 0; i < in->size; i++) {
        uint32_t kc = in->elements[i];
        if (!ng_is_left_key(kc)) {
            addToList(out, kc);
        }
    }
}





static int count_kana_entries(NGList *keys, bool exact_match) {
  if (keys->size == 0) return 0;

  int count = 0;
  uint32_t keyset0 = 0UL, keyset1 = 0UL, keyset2 = 0UL;
  
  // keysetを配列にしたらバイナリサイズが増えた
  switch (keys->size) {
    case 1:
      keyset0 = ng_key[keys->elements[0] - A];
      break;
    case 2:
      keyset0 = ng_key[keys->elements[0] - A];
      keyset1 = ng_key[keys->elements[1] - A];
      break;
    default:
      keyset0 = ng_key[keys->elements[0] - A];
      keyset1 = ng_key[keys->elements[1] - A];
      keyset2 = ng_key[keys->elements[2] - A];
      break;
  }

  for (int i = 0; i < sizeof ngdickana / sizeof ngdickana[i]; i++) {
    bool matches = false;

    switch (keys->size) {
      case 1:
        if (exact_match) {
          matches = (ngdickana[i].shift == keyset0) || 
                   (ngdickana[i].shift == 0UL && ngdickana[i].douji == keyset0);
        } else {
          matches = ((ngdickana[i].shift & keyset0) == keyset0) ||
                   (ngdickana[i].shift == 0UL && (ngdickana[i].douji & keyset0) == keyset0);
        }
        break;
      case 2:
        if (exact_match) {
          matches = (ngdickana[i].shift == (keyset0 | keyset1)) ||
                   (ngdickana[i].shift == keyset0 && ngdickana[i].douji == keyset1) ||
                   (ngdickana[i].shift == 0UL && ngdickana[i].douji == (keyset0 | keyset1));
        } else {
          matches = (ngdickana[i].shift == (keyset0 | keyset1)) ||
                   (ngdickana[i].shift == keyset0 && (ngdickana[i].douji & keyset1) == keyset1) ||
                   (ngdickana[i].shift == 0UL && (ngdickana[i].douji & (keyset0 | keyset1)) == (keyset0 | keyset1));
          // しぇ、ちぇ、など2キーで確定してはいけない
          if (matches && (ngdickana[i].shift | ngdickana[i].douji) != (keyset0 | keyset1)) {
            count = 2;
          }
        }
        break;
      default:
        if (exact_match) {
          matches = (ngdickana[i].shift == (keyset0 | keyset1) && ngdickana[i].douji == keyset2) ||
                   (ngdickana[i].shift == keyset0 && ngdickana[i].douji == (keyset1 | keyset2)) ||
                   (ngdickana[i].shift == 0UL && ngdickana[i].douji == (keyset0 | keyset1 | keyset2));
        } else {
          matches = (ngdickana[i].shift == (keyset0 | keyset1) && (ngdickana[i].douji & keyset2) == keyset2) ||
                   (ngdickana[i].shift == keyset0 && (ngdickana[i].douji & (keyset1 | keyset2)) == (keyset1 | keyset2)) ||
                   (ngdickana[i].shift == 0UL && (ngdickana[i].douji & (keyset0 | keyset1 | keyset2)) == (keyset0 | keyset1 | keyset2));
        }
        break;
    }

    if (matches) {
      count++;
      if (count > 1) break;
    }
  }

  return count;
}

int number_of_matches(NGList *keys) {  
  int result = count_kana_entries(keys, true);
  return result;
}

int number_of_candidates(NGList *keys) {
  int result = count_kana_entries(keys, false);
  return result;
}




// Mejiro 粒ベース: 左手 cvb, 右手 n m ,
static int mej_ntk_map(uint32_t kc, char *out_code) {
    switch (kc) {
        // 左手: C, V, B -> n, t, k
        case C:
            *out_code = 'n';
            return 1; // left
        case V:
            *out_code = 't';
            return 1;
        case B:
            *out_code = 'k';
            return 1;

        // 右手: N, M, COMMA -> n, t, k
        case COMMA:
            *out_code = 'n';
            return 2; // right
        case M:
            *out_code = 't';
            return 2;
        case N:
            *out_code = 'k';
            return 2;

        default:
            return 0; // Mejiro 粒ではない
    }
}

// このストロークが「cvb / n m , だけ」で構成されているか？
static bool mej_is_pure_ntk_stroke(NGList *keys) {
    if (keys->size == 0) {
        return false;
    }
    for (int i = 0; i < keys->size; i++) {
        char code;
        if (mej_ntk_map(keys->elements[i], &code) == 0) {
            return false;
        }
    }
    return true;
}



// Mejiro の ntk 例外マップをキーボード側で実装する
// 戻り値: 処理したら true（= ng_type() 本体はスキップしてよい）
static bool mej_handle_ntk_exception(NGList *keys) {
    bool l_n = false, l_t = false, l_k = false;
    bool r_n = false, r_t = false, r_k = false;

    // 左右の n/t/k を集計
    for (int i = 0; i < keys->size; i++) {
        uint32_t kc = keys->elements[i];
        char code;
        int side = mej_ntk_map(kc, &code);
        if (side == 0) {
            // 想定外が混ざっていたら何もしない
            return false;
        }

        if (side == 1) {
            if (code == 'n') l_n = true;
            if (code == 't') l_t = true;
            if (code == 'k') l_k = true;
        } else {
            if (code == 'n') r_n = true;
            if (code == 't') r_t = true;
            if (code == 'k') r_k = true;
        }
    }

    // ---- ここから Mejiro EXCEPTION_STROKE_MAP を ZMK に落とし込む ----
    // Python 側:
    // "-n":  Return
    // "n-":  Space
    // "n-n": Space + Return
    // "-nt": "。"
    // "-nk": "、"
    // "-ntk": "や"
    // "n-nt": "?"
    // "n-nk": "!"
    // "n-ntk": "や、"

    // 1) "-n" : 右 n のみ → Enter
    if (!l_n && !l_t && !l_k && r_n && !r_t && !r_k) {
        LOG_DBG(" MEJIRO -n => Enter");
        raise_zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp);
        return true;
    }

    // 2) "n-" : 左 n のみ → Space
    if (l_n && !l_t && !l_k && !r_n && !r_t && !r_k) {
        LOG_DBG(" MEJIRO n- => Space");
        raise_zmk_keycode_state_changed_from_encoded(SPACE, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp);
        return true;
    }

    // 3) "n-n" : 左 n + 右 n → Space + Enter
    if (l_n && !l_t && !l_k && r_n && !r_t && !r_k) {
        LOG_DBG(" MEJIRO n-n => Space + Enter");
        raise_zmk_keycode_state_changed_from_encoded(SPACE, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(SPACE, false, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp);
        return true;
    }

    // 4) "-nt" : 右 n+t → "。"
    if (!l_n && !l_t && !l_k && r_n && r_t && !r_k) {
        LOG_DBG(" MEJIRO -nt => 。");
        // すでに ngdickana に「。 + Space」のエントリがあるので、
        // ここでは単純に DOT を送るだけにしておく（変換はユーザの運用次第）
        raise_zmk_keycode_state_changed_from_encoded(DOT, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(DOT, false, timestamp);
        return true;
    }

    // 5) "-nk" : 右 n+k → "、"
    if (!l_n && !l_t && !l_k && r_n && !r_t && r_k) {
        LOG_DBG(" MEJIRO -nk => 、");
        raise_zmk_keycode_state_changed_from_encoded(COMMA, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(COMMA, false, timestamp);
        return true;
    }

    // 6) "-ntk" : 右 n+t+k → "や" （= "ya"）
    if (!l_n && !l_t && !l_k && r_n && r_t && r_k) {
        LOG_DBG(" MEJIRO -ntk => や (ya)");
        raise_zmk_keycode_state_changed_from_encoded(Y, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(Y, false, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(A, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(A, false, timestamp);
        return true;
    }

    // 7) "n-nt" : 左 n + 右 n+t → "?"
    if (l_n && !l_t && !l_k && r_n && r_t && !r_k) {
        LOG_DBG(" MEJIRO n-nt => ?");
        // ASCII ? を素直に送る（= SHIFT + SLASH 相当）
        raise_zmk_keycode_state_changed_from_encoded(QUESTION, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(QUESTION, false, timestamp);
        return true;
    }

    // 8) "n-nk" : 左 n + 右 n+k → "!"
    if (l_n && !l_t && !l_k && r_n && !r_t && r_k) {
        LOG_DBG(" MEJIRO n-nk => !");
        raise_zmk_keycode_state_changed_from_encoded(EXCLAMATION, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(EXCLAMATION, false, timestamp);
        return true;
    }

    // 9) "n-ntk" : 左 n + 右 n+t+k → "や、"
    if (l_n && !l_t && !l_k && r_n && r_t && r_k) {
        LOG_DBG(" MEJIRO n-ntk => や、");
        raise_zmk_keycode_state_changed_from_encoded(Y, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(Y, false, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(A, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(A, false, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(COMMA, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(COMMA, false, timestamp);
        return true;
    }

    // ここまでどれにも該当しなければ、Mejiro 例外ではない
    return false;
}







// キー入力を文字に変換して出力する
void ng_type(NGList *keys) {
    LOG_DBG(">NAGINATA NG_TYPE");

    if (keys->size == 0)
        return;

    /* Mejiro core: if enabled and recognized, handle here (do not fall through to Naginata). */
    if (mej_enabled) {
        if (mej_type_once(keys)) {
            return;
        }
    }

    if (keys->size == 1 && keys->elements[0] == ENTER) {
        LOG_DBG(" NAGINATA type keycode 0x%02X", ENTER);
        raise_zmk_keycode_state_changed_from_encoded(ENTER, true, timestamp);
        raise_zmk_keycode_state_changed_from_encoded(ENTER, false, timestamp);
        return;
    }


    // ★ ここで「cvb / n m , だけ」のストロークなら
    //    Mejiro ntk 例外ルールで出力して終わり
    if (mej_is_pure_ntk_stroke(keys)) {
        if (mej_handle_ntk_exception(keys)) {
            LOG_DBG("<NAGINATA NG_TYPE (mejiro ntk exception)");
            return;
        }
        // ここで false が返った場合は「ntk だけど、まだ定義してないパターン」
        // 将来的に L_PARTICLE / R_PARTICLE の実装を足す余地を残す
    }

    // ↓ここから先は従来の薙刀式ロジックをそのまま維持
    
    uint32_t keyset = 0UL;
    for (int i = 0; i < keys->size; i++) {
        keyset |= ng_key[keys->elements[i] - A];
    }

    for (int i = 0; i < sizeof ngdickana / sizeof ngdickana[0]; i++) {
        if ((ngdickana[i].shift | ngdickana[i].douji) == keyset) {
            if (ngdickana[i].kana[0] != NONE) {
                for (int k = 0; k < 6; k++) {
                    if (ngdickana[i].kana[k] == NONE)
                        break;
                    LOG_DBG(" NAGINATA type keycode 0x%02X", ngdickana[i].kana[k]);
                    raise_zmk_keycode_state_changed_from_encoded(ngdickana[i].kana[k], true,
                                                                 timestamp);
                    raise_zmk_keycode_state_changed_from_encoded(ngdickana[i].kana[k], false,
                                                                 timestamp);
                }
            } else {
                ngdickana[i].func();
            }
            LOG_DBG("<NAGINATA NG_TYPE");
            return;
        }
    }

    // JIみたいにJIを含む同時押しはたくさんあるが、JIのみの同時押しがないとき
    // 最後の１キーを別に分けて変換する
    NGList a, b;
    initializeList(&a);
    initializeList(&b);
    for (int i = 0; i < keys->size - 1; i++) {
        addToList(&a, keys->elements[i]);
    }
    addToList(&b, keys->elements[keys->size - 1]);
    ng_type(&a);
    ng_type(&b);

    LOG_DBG("<NAGINATA NG_TYPE");
}







// 薙刀式の入力処理
bool naginata_press(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    LOG_DBG(">NAGINATA PRESS");

    uint32_t keycode = binding->param1;

    switch (keycode) {
    case A ... Z:
    case SPACE:
    case ENTER:
    case DOT:
    case COMMA:
    case SLASH:
    case SEMI:
    case SQT:        
        n_pressed_keys++;
        pressed_keys |= ng_key[keycode - A]; // キーの重ね合わせ

        if (keycode == SPACE || keycode == ENTER) {
            NGList a;
            initializeList(&a);
            addToList(&a, keycode);
            addToListArray(&nginput, &a);
        } else {
            NGList a;
            NGList b;
            if (nginput.size > 0) {
                copyList(&(nginput.elements[nginput.size - 1]), &a);
                copyList(&a, &b);
                addToList(&b, keycode);
            }

            // 前のキーとの同時押しの可能性があるなら前に足す
            // 同じキー連打を除外
            if (nginput.size > 0 && a.elements[a.size - 1] != keycode &&
                number_of_candidates(&b) > 0) {
                removeFromListArrayAt(&nginput, nginput.size - 1);
                addToListArray(&nginput, &b);
                // 前のキーと同時押しはない
            } else {
                // 連続シフトではない
                NGList e;
                initializeList(&e);
                addToList(&e, keycode);
                addToListArray(&nginput, &e);
            }
        }

        // 連続シフト
        static uint32_t rs[10][2] = {{D, F},     {C, V}, {J, K}, {M, COMMA}, {SPACE, 0},
                                     {ENTER, 0}, {F, 0}, {V, 0}, {J, 0},     {M, 0}};

        uint32_t keyset = 0UL;
        for (int i = 0; i < nginput.elements[0].size; i++) {
            keyset |= ng_key[nginput.elements[0].elements[i] - A];
        }
        for (int i = 0; i < 10; i++) {
            NGList rskc;
            initializeList(&rskc);
            addToList(&rskc, rs[i][0]);
            if (rs[i][1] > 0) {
                addToList(&rskc, rs[i][1]);
            }

            int c = includeList(&rskc, keycode);
            uint32_t brs = 0UL;
            for (int j = 0; j < rskc.size; j++) {
                brs |= ng_key[rskc.elements[j] - A];
            }

            NGList l = nginput.elements[nginput.size - 1];
            for (int j = 0; j < l.size; j++) {
                addToList(&rskc, l.elements[j]);
            }

            if (c < 0 && ((brs & pressed_keys) == brs) && (keyset & brs) != brs && number_of_matches(&rskc) > 0) {
                nginput.elements[nginput.size - 1] = rskc;
                break;
            }
        }

        if (nginput.size > 1 || number_of_candidates(&(nginput.elements[0])) == 1) {
            ng_type(&(nginput.elements[0]));
            removeFromListArrayAt(&nginput, 0);
        }
        break;
    }

    LOG_DBG("<NAGINATA PRESS");

    return true;
}

bool naginata_release(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    LOG_DBG(">NAGINATA RELEASE");

    uint32_t keycode = binding->param1;

    switch (keycode) {
    case A ... Z:
    case SPACE:
    case ENTER:
    case DOT:
    case COMMA:
    case SLASH:
    case SEMI:
    case SQT:
        if (n_pressed_keys > 0)
            n_pressed_keys--;
        if (n_pressed_keys == 0)
            pressed_keys = 0UL;

        pressed_keys &= ~ng_key[keycode - A]; // キーの重ね合わせ

        if (pressed_keys == 0UL) {
            while (nginput.size > 0) {
                ng_type(&(nginput.elements[0]));
                removeFromListArrayAt(&nginput, 0);
            }
        } else {
            if (nginput.size > 0 && number_of_candidates(&(nginput.elements[0])) == 1) {
                ng_type(&(nginput.elements[0]));
                removeFromListArrayAt(&nginput, 0);
            }
        }
        break;
    }

    LOG_DBG("<NAGINATA RELEASE");

    return true;
}

// 薙刀式

static int behavior_naginata_init(const struct device *dev) {
    LOG_DBG("NAGINATA INIT");

    initializeListArray(&nginput);
    pressed_keys = 0UL;
    n_pressed_keys = 0;
    naginata_config.os =  NG_MACOS;

    return 0;
};

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d keycode 0x%02X", event.position, binding->param1);

    /* Mejiro enable/disable controls (bind these keycodes with &ng):
     *   F20: enable, F21: disable, F22: toggle
     */
    if (binding->param1 == F20) { mej_enabled = true;  mej_passthrough(F20, true, event.timestamp); return ZMK_BEHAVIOR_OPAQUE; }
    if (binding->param1 == F21) { mej_enabled = false; mej_passthrough(F21, true, event.timestamp); return ZMK_BEHAVIOR_OPAQUE; }
    if (binding->param1 == F22) { mej_enabled = !mej_enabled; mej_passthrough(F22, true, event.timestamp); return ZMK_BEHAVIOR_OPAQUE; }

    /* If Mejiro is disabled, behave like normal key press (passthrough). */
    if (!mej_enabled) {
        mej_passthrough(binding->param1, true, event.timestamp);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    // F15が押されたらnaginata_config.os=NG_WINDOWS
    switch (binding->param1) {
        case F15:
            naginata_config.os = NG_WINDOWS;
            return ZMK_BEHAVIOR_OPAQUE;
        case F16:
            naginata_config.os = NG_MACOS;
            return ZMK_BEHAVIOR_OPAQUE;
        case F17:
            naginata_config.os = NG_LINUX;
            return ZMK_BEHAVIOR_OPAQUE;
        case F18:
            naginata_config.tategaki = true;
            return ZMK_BEHAVIOR_OPAQUE;
        case F19:
            naginata_config.tategaki = false;
            return ZMK_BEHAVIOR_OPAQUE;
    }

    timestamp = event.timestamp;
    naginata_press(binding, event);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d keycode 0x%02X", event.position, binding->param1);

    /* When disabled, behave like normal key release (passthrough). */
    if (!mej_enabled) {
        mej_passthrough(binding->param1, false, event.timestamp);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    timestamp = event.timestamp;
    naginata_release(binding, event);

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_naginata_driver_api = {
    .binding_pressed = on_keymap_binding_pressed, .binding_released = on_keymap_binding_released};

#define KP_INST(n)                                                                                 \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_naginata_init, NULL, NULL, NULL, POST_KERNEL,              \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_naginata_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_INST)
