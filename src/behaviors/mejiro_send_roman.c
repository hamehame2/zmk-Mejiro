/*
 * SPDX-License-Identifier: MIT
 *
 * mejiro_send_roman.c
 * - Plover辞書っぽい "{...}" マクロ表記の最小サポート
 * - ZMK の HID API に対して「HID usage(数値)」を直接投げる
 *
 * 目的:
 *  - src/behaviors/mejiro_tables.c の mj_commands[] に入っている
 *    "{#BackSpace}" "{#Left}" "{^[]^}{#Left}{^}" などを送れるようにする
 *
 * 注意:
 *  - "{^}" は Plover の “attach” 系(空の attach) とみなして no-op
 *  - "{^...^}" は “そのまま文字列を打鍵” として扱う（US配列想定の簡易ASCII→HID変換）
 *  - "{#XXXX}" は特殊キーを送る
 *  - "=undo" など "=" で始まる抽象コマンドは、このファイルでは未実装（false返す）
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/endpoints.h>
#include <zmk/hid.h>

/* もしヘッダがあるなら使ってOKだが、環境差で無い/名前が違うことがあるので、
 * このファイルは “完全自前定義” でビルドが通るようにする。
 */
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* -------------------------------------------------------------------------- */
/* HID usage: Keyboard (US)                                                   */
/* 参考: USB HID Usage Tables (Keyboard/Keypad Page 0x07)                     */
/* ZMKの zmk_hid_keyboard_press/release は usage(0x04.. など) を受け取る前提で使う */
/* -------------------------------------------------------------------------- */

/* Modifiers (0xE0..0xE7) */
#define HID_KC_LCTRL  0xE0
#define HID_KC_LSHIFT 0xE1
#define HID_KC_LALT   0xE2
#define HID_KC_LGUI   0xE3
#define HID_KC_RCTRL  0xE4
#define HID_KC_RSHIFT 0xE5
#define HID_KC_RALT   0xE6
#define HID_KC_RGUI   0xE7

/* Control / navigation */
#define HID_KC_ENTER      0x28
#define HID_KC_ESCAPE     0x29
#define HID_KC_BACKSPACE  0x2A
#define HID_KC_TAB        0x2B
#define HID_KC_SPACE      0x2C
#define HID_KC_DELETE     0x4C
#define HID_KC_HOME       0x4A
#define HID_KC_END        0x4D
#define HID_KC_LEFT       0x50
#define HID_KC_RIGHT      0x4F
#define HID_KC_UP         0x52
#define HID_KC_DOWN       0x51

/* Function keys (F1=0x3A ...) */
#define HID_KC_F13        0x68
#define HID_KC_F14        0x69
#define HID_KC_F15        0x6A
#define HID_KC_F16        0x6B
#define HID_KC_F17        0x6C
#define HID_KC_F18        0x6D
#define HID_KC_F19        0x6E
#define HID_KC_F20        0x6F
#define HID_KC_F21        0x70
#define HID_KC_F22        0x71
#define HID_KC_F23        0x72
#define HID_KC_F24        0x73

/* Letters */
#define HID_KC_A 0x04
#define HID_KC_B 0x05
#define HID_KC_C 0x06
#define HID_KC_D 0x07
#define HID_KC_E 0x08
#define HID_KC_F 0x09
#define HID_KC_G 0x0A
#define HID_KC_H 0x0B
#define HID_KC_I 0x0C
#define HID_KC_J 0x0D
#define HID_KC_K 0x0E
#define HID_KC_L 0x0F
#define HID_KC_M 0x10
#define HID_KC_N 0x11
#define HID_KC_O 0x12
#define HID_KC_P 0x13
#define HID_KC_Q 0x14
#define HID_KC_R 0x15
#define HID_KC_S 0x16
#define HID_KC_T 0x17
#define HID_KC_U 0x18
#define HID_KC_V 0x19
#define HID_KC_W 0x1A
#define HID_KC_X 0x1B
#define HID_KC_Y 0x1C
#define HID_KC_Z 0x1D

/* Digits row */
#define HID_KC_1 0x1E
#define HID_KC_2 0x1F
#define HID_KC_3 0x20
#define HID_KC_4 0x21
#define HID_KC_5 0x22
#define HID_KC_6 0x23
#define HID_KC_7 0x24
#define HID_KC_8 0x25
#define HID_KC_9 0x26
#define HID_KC_0 0x27

/* Punctuation (US) */
#define HID_KC_MINUS       0x2D
#define HID_KC_EQUAL       0x2E
#define HID_KC_LBRACKET    0x2F
#define HID_KC_RBRACKET    0x30
#define HID_KC_BSLASH      0x31
#define HID_KC_SEMICOLON   0x33
#define HID_KC_QUOTE       0x34
#define HID_KC_GRAVE       0x35
#define HID_KC_COMMA       0x36
#define HID_KC_DOT         0x37
#define HID_KC_SLASH       0x38

/* -------------------------------------------------------------------------- */
/* Low-level send                                                             */
/* -------------------------------------------------------------------------- */

static void tap(uint16_t keycode) {
    zmk_hid_keyboard_press(zmk_endpoints_selected(), keycode);
    zmk_hid_keyboard_release(zmk_endpoints_selected(), keycode);
}

static void tap_with_mod(uint16_t mod_keycode, uint16_t keycode) {
    zmk_hid_keyboard_press(zmk_endpoints_selected(), mod_keycode);
    zmk_hid_keyboard_press(zmk_endpoints_selected(), keycode);
    zmk_hid_keyboard_release(zmk_endpoints_selected(), keycode);
    zmk_hid_keyboard_release(zmk_endpoints_selected(), mod_keycode);
}

/* -------------------------------------------------------------------------- */
/* ASCII -> HID (US) minimal mapper                                           */
/* ここは “必要な範囲を全部書く” 方式にして、後から拡張できるようにする              */
/* -------------------------------------------------------------------------- */

struct hid_key {
    uint16_t mod;   /* 0 or HID_KC_LSHIFT など */
    uint16_t code;  /* HID usage */
};

static bool map_ascii_us(char c, struct hid_key *out) {
    if (!out) {
        return false;
    }

    out->mod = 0;
    out->code = 0;

    /* a-z */
    if (c >= 'a' && c <= 'z') {
        out->code = (uint16_t)(HID_KC_A + (c - 'a'));
        return true;
    }

    /* A-Z */
    if (c >= 'A' && c <= 'Z') {
        out->mod = HID_KC_LSHIFT;
        out->code = (uint16_t)(HID_KC_A + (c - 'A'));
        return true;
    }

    /* 0-9 */
    if (c >= '1' && c <= '9') {
        out->code = (uint16_t)(HID_KC_1 + (c - '1'));
        return true;
    }
    if (c == '0') {
        out->code = HID_KC_0;
        return true;
    }

    /* Space */
    if (c == ' ') {
        out->code = HID_KC_SPACE;
        return true;
    }

    /* Common punctuation */
    switch (c) {
    case '-': out->code = HID_KC_MINUS; return true;
    case '_': out->mod = HID_KC_LSHIFT; out->code = HID_KC_MINUS; return true;

    case '=': out->code = HID_KC_EQUAL; return true;
    case '+': out->mod = HID_KC_LSHIFT; out->code = HID_KC_EQUAL; return true;

    case '[': out->code = HID_KC_LBRACKET; return true;
    case '{': out->mod = HID_KC_LSHIFT; out->code = HID_KC_LBRACKET; return true;

    case ']': out->code = HID_KC_RBRACKET; return true;
    case '}': out->mod = HID_KC_LSHIFT; out->code = HID_KC_RBRACKET; return true;

    case '\\': out->code = HID_KC_BSLASH; return true;
    case '|':  out->mod = HID_KC_LSHIFT; out->code = HID_KC_BSLASH; return true;

    case ';': out->code = HID_KC_SEMICOLON; return true;
    case ':': out->mod = HID_KC_LSHIFT; out->code = HID_KC_SEMICOLON; return true;

    case '\'': out->code = HID_KC_QUOTE; return true;
    case '"':  out->mod = HID_KC_LSHIFT; out->code = HID_KC_QUOTE; return true;

    case '`': out->code = HID_KC_GRAVE; return true;
    case '~': out->mod = HID_KC_LSHIFT; out->code = HID_KC_GRAVE; return true;

    case ',': out->code = HID_KC_COMMA; return true;
    case '<': out->mod = HID_KC_LSHIFT; out->code = HID_KC_COMMA; return true;

    case '.': out->code = HID_KC_DOT; return true;
    case '>': out->mod = HID_KC_LSHIFT; out->code = HID_KC_DOT; return true;

    case '/': out->code = HID_KC_SLASH; return true;
    case '?': out->mod = HID_KC_LSHIFT; out->code = HID_KC_SLASH; return true;

    /* Digits with shift symbols */
    case '!': out->mod = HID_KC_LSHIFT; out->code = HID_KC_1; return true;
    case '@': out->mod = HID_KC_LSHIFT; out->code = HID_KC_2; return true;
    case '#': out->mod = HID_KC_LSHIFT; out->code = HID_KC_3; return true;
    case '$': out->mod = HID_KC_LSHIFT; out->code = HID_KC_4; return true;
    case '%': out->mod = HID_KC_LSHIFT; out->code = HID_KC_5; return true;
    case '^': out->mod = HID_KC_LSHIFT; out->code = HID_KC_6; return true;
    case '&': out->mod = HID_KC_LSHIFT; out->code = HID_KC_7; return true;
    case '*': out->mod = HID_KC_LSHIFT; out->code = HID_KC_8; return true;
    case '(': out->mod = HID_KC_LSHIFT; out->code = HID_KC_9; return true;
    case ')': out->mod = HID_KC_LSHIFT; out->code = HID_KC_0; return true;

    default:
        break;
    }

    return false;
}

static void type_ascii_us(const char *s, size_t len) {
    if (!s || len == 0) {
        return;
    }

    for (size_t i = 0; i < len; i++) {
        struct hid_key hk;
        if (!map_ascii_us(s[i], &hk)) {
            /* 未対応文字は黙って捨てる（必要なら LOG_DBG にしてもOK） */
            continue;
        }
        if (hk.mod) {
            tap_with_mod(hk.mod, hk.code);
        } else {
            tap(hk.code);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* "{#NAME}" special key handler                                              */
/* -------------------------------------------------------------------------- */

static bool exec_hash_word(const char *name, size_t nlen) {
    if (!name || nlen == 0) {
        return false;
    }

    /* 文字列比較（全部手書きで） */
    #define IS_LIT(lit) (nlen == (sizeof(lit) - 1) && (memcmp(name, lit, sizeof(lit) - 1) == 0))

    if (IS_LIT("BackSpace") || IS_LIT("Backspace")) { tap(HID_KC_BACKSPACE); return true; }
    if (IS_LIT("Delete"))                           { tap(HID_KC_DELETE);    return true; }
    if (IS_LIT("Left"))                             { tap(HID_KC_LEFT);      return true; }
    if (IS_LIT("Right"))                            { tap(HID_KC_RIGHT);     return true; }
    if (IS_LIT("Up"))                               { tap(HID_KC_UP);        return true; }
    if (IS_LIT("Down"))                             { tap(HID_KC_DOWN);      return true; }
    if (IS_LIT("Home"))                             { tap(HID_KC_HOME);      return true; }
    if (IS_LIT("End"))                              { tap(HID_KC_END);       return true; }
    if (IS_LIT("Escape") || IS_LIT("Esc"))           { tap(HID_KC_ESCAPE);    return true; }
    if (IS_LIT("Enter") || IS_LIT("Return"))         { tap(HID_KC_ENTER);     return true; }
    if (IS_LIT("Tab"))                              { tap(HID_KC_TAB);       return true; }

    /* F13/F14 は mj_commands に出ている */
    if (IS_LIT("F13"))                              { tap(HID_KC_F13);       return true; }
    if (IS_LIT("F14"))                              { tap(HID_KC_F14);       return true; }

    #undef IS_LIT
    return false;
}

/* -------------------------------------------------------------------------- */
/* Plover-ish macro string executor                                           */
/* 例: "{^[]^}{#Left}{^}"                                                     */
/* -------------------------------------------------------------------------- */

static bool exec_brace_payload(const char *payload, size_t plen) {
    if (!payload) {
        return false;
    }
    if (plen == 0) {
        return true; /* "{}" は no-op 扱い */
    }

    /* "{^}" ＝ attach/no-op */
    if (plen == 1 && payload[0] == '^') {
        return true;
    }

    /* "{#XXXX}" ＝ special key */
    if (payload[0] == '#') {
        return exec_hash_word(payload + 1, plen - 1);
    }

    /* "{^...^}" ＝ literal text */
    if (payload[0] == '^' && payload[plen - 1] == '^') {
        if (plen >= 2) {
            type_ascii_us(payload + 1, plen - 2);
            return true;
        }
        return true;
    }

    /* "=undo" などをここで解釈するなら payload[0]=='=' を処理。
     * ただし今回は “出力(打鍵)担当” に絞るので未実装にして false。
     */
    if (payload[0] == '=') {
        return false;
    }

    /* その他: そのまま文字として打つ（保険） */
    type_ascii_us(payload, plen);
    return true;
}

static bool exec_macro_string(const char *s) {
    if (!s) {
        return false;
    }

    /* 先頭が '=' の抽象コマンドは未実装 */
    if (s[0] == '=') {
        return false;
    }

    const char *p = s;

    while (*p) {
        if (*p == '{') {
            const char *start = p + 1;
            const char *q = start;

            while (*q && *q != '}') {
                q++;
            }
            if (*q != '}') {
                /* '}' が無い -> 以降を通常テキストとして処理 */
                type_ascii_us(p, strlen(p));
                return true;
            }

            /* payload: [start, q) */
            (void)exec_brace_payload(start, (size_t)(q - start));
            p = q + 1;
            continue;
        }

        /* 通常文字はそのまま打鍵 */
        struct hid_key hk;
        if (map_ascii_us(*p, &hk)) {
            if (hk.mod) {
                tap_with_mod(hk.mod, hk.code);
            } else {
                tap(hk.code);
            }
        }
        p++;
    }

    return true;
}

/* -------------------------------------------------------------------------- */
/* Public API (core から呼ぶ想定)                                             */
/* -------------------------------------------------------------------------- */

/* core 側が「辞書展開済みの文字列」をここへ渡してくる想定。
 * 返り値:
 *  - true: 何かしら送った
 *  - false: ここでは処理しない（上位で別処理へ）
 */
bool mejiro_send_roman(const char *macro_or_text) {
    if (!macro_or_text || macro_or_text[0] == '\0') {
        return false;
    }

    /* mj_commands の値は "{...}" を含むので、ここで実行 */
    return exec_macro_string(macro_or_text);
}
