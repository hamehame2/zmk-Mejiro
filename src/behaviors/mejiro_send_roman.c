/*
 * Mejiro: send roman / raw key tapping helper
 *
 * Goal:
 *  - Convert output strings (roman) into HID keyboard key taps on the device side.
 *  - Avoid including Zephyr USB HID dt-bindings header (not present in this build).
 *  - Work with ZMK endpoints API: zmk_endpoints_selected() returns a struct value
 *    in your environment, so we store it and pass &ep to HID functions.
 *
 * Notes:
 *  - This file intentionally does NOT include <zephyr/dt-bindings/usb/hid.h>.
 *  - We only rely on ZMK headers.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <zmk/endpoints.h>
#include <zmk/hid.h>
#include <zmk/hid_keyboard.h>

#include "mejiro_send_roman.h"

/* -------------------------------------------------------------------------- */
/* Timing (tiny gaps help some hosts and keep ordering stable)                 */
/* -------------------------------------------------------------------------- */

#ifndef MJ_TAP_DELAY_MS
#define MJ_TAP_DELAY_MS 1
#endif

#ifndef MJ_SEQ_DELAY_MS
#define MJ_SEQ_DELAY_MS 0
#endif

static inline void mj_sleep_ms(int ms) {
    if (ms > 0) {
        k_sleep(K_MSEC(ms));
    }
}

/* -------------------------------------------------------------------------- */
/* Keycode encoding helpers                                                    */
/* -------------------------------------------------------------------------- */

/*
 * ZMK keycodes:
 *  - For "normal" keys we use ZMK_HID_USAGE(HID_USAGE_KEY, <usage_id>).
 *  - For modifiers we use zmk_hid_keyboard_press/release_modifiers().
 *
 * If your ZMK version uses different helper names for modifiers,
 * this is the single place you’ll edit.
 */

/* Common HID usage IDs for keyboard (USB HID Usage Tables) */
enum {
    HID_KBD_A = 0x04,
    HID_KBD_B = 0x05,
    HID_KBD_C = 0x06,
    HID_KBD_D = 0x07,
    HID_KBD_E = 0x08,
    HID_KBD_F = 0x09,
    HID_KBD_G = 0x0A,
    HID_KBD_H = 0x0B,
    HID_KBD_I = 0x0C,
    HID_KBD_J = 0x0D,
    HID_KBD_K = 0x0E,
    HID_KBD_L = 0x0F,
    HID_KBD_M = 0x10,
    HID_KBD_N = 0x11,
    HID_KBD_O = 0x12,
    HID_KBD_P = 0x13,
    HID_KBD_Q = 0x14,
    HID_KBD_R = 0x15,
    HID_KBD_S = 0x16,
    HID_KBD_T = 0x17,
    HID_KBD_U = 0x18,
    HID_KBD_V = 0x19,
    HID_KBD_W = 0x1A,
    HID_KBD_X = 0x1B,
    HID_KBD_Y = 0x1C,
    HID_KBD_Z = 0x1D,

    HID_KBD_1 = 0x1E,
    HID_KBD_2 = 0x1F,
    HID_KBD_3 = 0x20,
    HID_KBD_4 = 0x21,
    HID_KBD_5 = 0x22,
    HID_KBD_6 = 0x23,
    HID_KBD_7 = 0x24,
    HID_KBD_8 = 0x25,
    HID_KBD_9 = 0x26,
    HID_KBD_0 = 0x27,

    HID_KBD_ENTER      = 0x28,
    HID_KBD_ESCAPE     = 0x29,
    HID_KBD_BACKSPACE  = 0x2A,
    HID_KBD_TAB        = 0x2B,
    HID_KBD_SPACE      = 0x2C,

    HID_KBD_MINUS      = 0x2D, /* - _ */
    HID_KBD_EQUAL      = 0x2E, /* = + */
    HID_KBD_LBRACKET   = 0x2F, /* [ { */
    HID_KBD_RBRACKET   = 0x30, /* ] } */
    HID_KBD_BSLASH     = 0x31, /* \ | */
    HID_KBD_SEMICOLON  = 0x33, /* ; : */
    HID_KBD_APOSTROPHE = 0x34, /* ' " */
    HID_KBD_GRAVE      = 0x35, /* ` ~ */
    HID_KBD_COMMA      = 0x36, /* , < */
    HID_KBD_DOT        = 0x37, /* . > */
    HID_KBD_SLASH      = 0x38, /* / ? */

    HID_KBD_CAPSLOCK   = 0x39,

    HID_KBD_F1         = 0x3A,
    HID_KBD_F2         = 0x3B,
    HID_KBD_F3         = 0x3C,
    HID_KBD_F4         = 0x3D,
    HID_KBD_F5         = 0x3E,
    HID_KBD_F6         = 0x3F,
    HID_KBD_F7         = 0x40,
    HID_KBD_F8         = 0x41,
    HID_KBD_F9         = 0x42,
    HID_KBD_F10        = 0x43,
    HID_KBD_F11        = 0x44,
    HID_KBD_F12        = 0x45,

    HID_KBD_RIGHT      = 0x4F,
    HID_KBD_LEFT       = 0x50,
    HID_KBD_DOWN       = 0x51,
    HID_KBD_UP         = 0x52,

    HID_KBD_HOME       = 0x4A,
    HID_KBD_END        = 0x4D,
    HID_KBD_DELETE     = 0x4C,
};

/* ZMK modifier bits (left shift etc) */
#ifndef MOD_LSFT
#define MOD_LSFT ZMK_HID_KEYBOARD_MODIFIER_LEFT_SHIFT
#endif
#ifndef MOD_LCTL
#define MOD_LCTL ZMK_HID_KEYBOARD_MODIFIER_LEFT_CTRL
#endif
#ifndef MOD_LALT
#define MOD_LALT ZMK_HID_KEYBOARD_MODIFIER_LEFT_ALT
#endif
#ifndef MOD_LGUI
#define MOD_LGUI ZMK_HID_KEYBOARD_MODIFIER_LEFT_GUI
#endif

static inline uint32_t mj_key_usage(uint8_t usage_id) {
    return ZMK_HID_USAGE(HID_USAGE_KEY, usage_id);
}

/* -------------------------------------------------------------------------- */
/* Low-level tapping                                                           */
/* -------------------------------------------------------------------------- */

static void mj_press_mods(uint8_t mods) {
    if (mods == 0) {
        return;
    }
    struct zmk_endpoint_instance ep = zmk_endpoints_selected();
    (void)zmk_hid_keyboard_press_modifiers(&ep, mods);
    (void)zmk_endpoints_send_report();
}

static void mj_release_mods(uint8_t mods) {
    if (mods == 0) {
        return;
    }
    struct zmk_endpoint_instance ep = zmk_endpoints_selected();
    (void)zmk_hid_keyboard_release_modifiers(&ep, mods);
    (void)zmk_endpoints_send_report();
}

static void mj_press_key(uint32_t keycode) {
    struct zmk_endpoint_instance ep = zmk_endpoints_selected();
    (void)zmk_hid_keyboard_press(&ep, keycode);
    (void)zmk_endpoints_send_report();
}

static void mj_release_key(uint32_t keycode) {
    struct zmk_endpoint_instance ep = zmk_endpoints_selected();
    (void)zmk_hid_keyboard_release(&ep, keycode);
    (void)zmk_endpoints_send_report();
}

static void mj_tap_key(uint32_t keycode, uint8_t mods) {
    /* Press modifiers -> press key -> release key -> release modifiers */
    if (mods) {
        mj_press_mods(mods);
        mj_sleep_ms(MJ_TAP_DELAY_MS);
    }

    mj_press_key(keycode);
    mj_sleep_ms(MJ_TAP_DELAY_MS);
    mj_release_key(keycode);

    if (mods) {
        mj_sleep_ms(MJ_TAP_DELAY_MS);
        mj_release_mods(mods);
    }

    mj_sleep_ms(MJ_SEQ_DELAY_MS);
}

/* -------------------------------------------------------------------------- */
/* ASCII -> (key, mods) mapping                                                */
/* -------------------------------------------------------------------------- */

struct mj_key_with_mods {
    uint32_t keycode;
    uint8_t mods;
    bool ok;
};

static struct mj_key_with_mods mj_map_ascii(char c) {
    struct mj_key_with_mods out = {0};

    /* a-z */
    if (c >= 'a' && c <= 'z') {
        uint8_t usage = (uint8_t)(HID_KBD_A + (c - 'a'));
        out.keycode = mj_key_usage(usage);
        out.mods = 0;
        out.ok = true;
        return out;
    }

    /* A-Z (with shift) */
    if (c >= 'A' && c <= 'Z') {
        uint8_t usage = (uint8_t)(HID_KBD_A + (c - 'A'));
        out.keycode = mj_key_usage(usage);
        out.mods = MOD_LSFT;
        out.ok = true;
        return out;
    }

    /* 0-9 */
    if (c >= '1' && c <= '9') {
        uint8_t usage = (uint8_t)(HID_KBD_1 + (c - '1'));
        out.keycode = mj_key_usage(usage);
        out.mods = 0;
        out.ok = true;
        return out;
    }
    if (c == '0') {
        out.keycode = mj_key_usage(HID_KBD_0);
        out.mods = 0;
        out.ok = true;
        return out;
    }

    /* whitespace / control-ish */
    switch (c) {
    case ' ':
        out.keycode = mj_key_usage(HID_KBD_SPACE);
        out.mods = 0;
        out.ok = true;
        return out;
    case '\n':
        out.keycode = mj_key_usage(HID_KBD_ENTER);
        out.mods = 0;
        out.ok = true;
        return out;
    case '\t':
        out.keycode = mj_key_usage(HID_KBD_TAB);
        out.mods = 0;
        out.ok = true;
        return out;
    default:
        break;
    }

    /* punctuation (US layout basis) */
    switch (c) {
    case '-': out.keycode = mj_key_usage(HID_KBD_MINUS); out.mods = 0; out.ok = true; return out;
    case '_': out.keycode = mj_key_usage(HID_KBD_MINUS); out.mods = MOD_LSFT; out.ok = true; return out;

    case '=': out.keycode = mj_key_usage(HID_KBD_EQUAL); out.mods = 0; out.ok = true; return out;
    case '+': out.keycode = mj_key_usage(HID_KBD_EQUAL); out.mods = MOD_LSFT; out.ok = true; return out;

    case '[': out.keycode = mj_key_usage(HID_KBD_LBRACKET); out.mods = 0; out.ok = true; return out;
    case '{': out.keycode = mj_key_usage(HID_KBD_LBRACKET); out.mods = MOD_LSFT; out.ok = true; return out;

    case ']': out.keycode = mj_key_usage(HID_KBD_RBRACKET); out.mods = 0; out.ok = true; return out;
    case '}': out.keycode = mj_key_usage(HID_KBD_RBRACKET); out.mods = MOD_LSFT; out.ok = true; return out;

    case '\\': out.keycode = mj_key_usage(HID_KBD_BSLASH); out.mods = 0; out.ok = true; return out;
    case '|':  out.keycode = mj_key_usage(HID_KBD_BSLASH); out.mods = MOD_LSFT; out.ok = true; return out;

    case ';': out.keycode = mj_key_usage(HID_KBD_SEMICOLON); out.mods = 0; out.ok = true; return out;
    case ':': out.keycode = mj_key_usage(HID_KBD_SEMICOLON); out.mods = MOD_LSFT; out.ok = true; return out;

    case '\'': out.keycode = mj_key_usage(HID_KBD_APOSTROPHE); out.mods = 0; out.ok = true; return out;
    case '"':  out.keycode = mj_key_usage(HID_KBD_APOSTROPHE); out.mods = MOD_LSFT; out.ok = true; return out;

    case '`': out.keycode = mj_key_usage(HID_KBD_GRAVE); out.mods = 0; out.ok = true; return out;
    case '~': out.keycode = mj_key_usage(HID_KBD_GRAVE); out.mods = MOD_LSFT; out.ok = true; return out;

    case ',': out.keycode = mj_key_usage(HID_KBD_COMMA); out.mods = 0; out.ok = true; return out;
    case '<': out.keycode = mj_key_usage(HID_KBD_COMMA); out.mods = MOD_LSFT; out.ok = true; return out;

    case '.': out.keycode = mj_key_usage(HID_KBD_DOT); out.mods = 0; out.ok = true; return out;
    case '>': out.keycode = mj_key_usage(HID_KBD_DOT); out.mods = MOD_LSFT; out.ok = true; return out;

    case '/': out.keycode = mj_key_usage(HID_KBD_SLASH); out.mods = 0; out.ok = true; return out;
    case '?': out.keycode = mj_key_usage(HID_KBD_SLASH); out.mods = MOD_LSFT; out.ok = true; return out;

    /* Number row symbols (US) */
    case '!': out.keycode = mj_key_usage(HID_KBD_1); out.mods = MOD_LSFT; out.ok = true; return out;
    case '@': out.keycode = mj_key_usage(HID_KBD_2); out.mods = MOD_LSFT; out.ok = true; return out;
    case '#': out.keycode = mj_key_usage(HID_KBD_3); out.mods = MOD_LSFT; out.ok = true; return out;
    case '$': out.keycode = mj_key_usage(HID_KBD_4); out.mods = MOD_LSFT; out.ok = true; return out;
    case '%': out.keycode = mj_key_usage(HID_KBD_5); out.mods = MOD_LSFT; out.ok = true; return out;
    case '^': out.keycode = mj_key_usage(HID_KBD_6); out.mods = MOD_LSFT; out.ok = true; return out;
    case '&': out.keycode = mj_key_usage(HID_KBD_7); out.mods = MOD_LSFT; out.ok = true; return out;
    case '*': out.keycode = mj_key_usage(HID_KBD_8); out.mods = MOD_LSFT; out.ok = true; return out;
    case '(': out.keycode = mj_key_usage(HID_KBD_9); out.mods = MOD_LSFT; out.ok = true; return out;
    case ')': out.keycode = mj_key_usage(HID_KBD_0); out.mods = MOD_LSFT; out.ok = true; return out;

    default:
        break;
    }

    out.ok = false;
    return out;
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                  */
/* -------------------------------------------------------------------------- */

void mj_send_ascii_string(const char *s) {
    if (!s) {
        return;
    }
    for (const char *p = s; *p; p++) {
        struct mj_key_with_mods km = mj_map_ascii(*p);
        if (!km.ok) {
            /* unknown char => skip */
            continue;
        }
        mj_tap_key(km.keycode, km.mods);
    }
}

/*
 * Convenience: send “special keys” that your tables may request
 * (e.g. {#Left}, {#Right}, etc.) as discrete taps.
 *
 * You can call these from your command parser (mejiro_core) if you want.
 */
void mj_tap_left(void)      { mj_tap_key(mj_key_usage(HID_KBD_LEFT), 0); }
void mj_tap_right(void)     { mj_tap_key(mj_key_usage(HID_KBD_RIGHT), 0); }
void mj_tap_up(void)        { mj_tap_key(mj_key_usage(HID_KBD_UP), 0); }
void mj_tap_down(void)      { mj_tap_key(mj_key_usage(HID_KBD_DOWN), 0); }
void mj_tap_home(void)      { mj_tap_key(mj_key_usage(HID_KBD_HOME), 0); }
void mj_tap_end(void)       { mj_tap_key(mj_key_usage(HID_KBD_END), 0); }
void mj_tap_escape(void)    { mj_tap_key(mj_key_usage(HID_KBD_ESCAPE), 0); }
void mj_tap_backspace(void) { mj_tap_key(mj_key_usage(HID_KBD_BACKSPACE), 0); }
void mj_tap_delete(void)    { mj_tap_key(mj_key_usage(HID_KBD_DELETE), 0); }
void mj_tap_enter(void)     { mj_tap_key(mj_key_usage(HID_KBD_ENTER), 0); }
