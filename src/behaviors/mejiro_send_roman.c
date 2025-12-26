/*
 * mejiro_send_roman.c
 *
 * Minimal "send string / key sequence" layer for Mejiro.
 * - DOES NOT include zmk/hid_keyboard.h (ZMK 4.1+ で死にやすい)
 * - Sends keys by raising keycode_state_changed events
 *
 * Supported mini-syntax (subset inspired by Plover):
 *   - Plain text: "abc", "Hello"
 *   - {#Left} {#Right} {#Up} {#Down} {#Home} {#End}
 *   - {#BackSpace} {#Delete} {#Enter} {#Tab} {#Escape}
 *   - {#F1}..{#F24}
 *   - {^literal^}  -> send literal string inside
 *   - "=undo"      -> send Ctrl+Z
 *
 * Anything unknown is treated as plain text (best-effort).
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/events/keycode_state_changed.h>
#include <zmk/event_manager.h>
/* ZMK keycode definitions (A, N1, LSHFT, etc.) */
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/modifiers.h>
#include <string.h>
#include <stdint.h>
#include <zmk/hid.h>                 // 既にあるならそのまま

#include <zmk_naginata/nglistarray.h>
#include <zephyr/sys/util.h>   /* ARG_UNUSED */



LOG_MODULE_REGISTER(mejiro_send_roman, CONFIG_ZMK_LOG_LEVEL);

/* -------------------------------------------------------------------------- */
/* QMK-style KC_* compatibility layer (minimal set used in this file)          */
/* -------------------------------------------------------------------------- */
/* Letters (HID contiguous) */
#define KC_A A

/* Digits (ZMK uses N1..N0 names) */
#define KC_1 N1
#define KC_2 N2
#define KC_3 N3
#define KC_4 N4
#define KC_5 N5
#define KC_6 N6
#define KC_7 N7
#define KC_8 N8
#define KC_9 N9
#define KC_0 N0

/* Whitespace / control */
#define KC_SPACE SPACE
#define KC_ENTER ENTER
#define KC_TAB   TAB
#define KC_ESCAPE ESC
#define KC_BSPC  BSPC
#define KC_DEL   DEL

/* Arrows / nav */
#define KC_LEFT  LEFT
#define KC_RIGHT RIGHT
#define KC_UP    UP
#define KC_DOWN  DOWN
#define KC_HOME  HOME
#define KC_END   END

/* Modifiers */
#define KC_LSHIFT LSHFT
#define KC_LCTRL  LCTRL

/* Punctuation (US) */
#define KC_MINUS MINUS
#define KC_EQUAL EQUAL
#define KC_LBKT  LBKT
#define KC_RBKT  RBKT
#define KC_BSLH  BSLH
#define KC_SEMI  SEMI
#define KC_SQT   SQT
#define KC_COMMA COMMA
#define KC_DOT   DOT
#define KC_FSLH  FSLH
#define KC_GRAVE GRAVE

/* Function keys */
#define KC_F1 F1
/* QMK-style compatibility (minimum set) */
#ifndef KC_Z
#define KC_Z Z
#endif
#ifndef KC_LCTRL
#define KC_LCTRL LCTRL
#endif
/* -------------------------------------------------------------------------- */
/* Key event helpers                                                          */
/* -------------------------------------------------------------------------- */

static inline int64_t now_ms(void) {
    /* uptime in ms is fine for event timestamps */
    return (int64_t)k_uptime_get();
}

/*
 * Raise a "keycode changed" event with encoded keycode.
 * This relies on ZMK's normal pipeline to deliver HID/USB/BLE output.
 */
static void send_keycode_event(uint32_t keycode, bool pressed) {
    int64_t ts = now_ms();
    /* In ZMK 4.x the helper is typically available. If your tree uses a different
     * name, this is the only place you need to adjust. */
    raise_zmk_keycode_state_changed_from_encoded(keycode, pressed, ts);
}

static void tap_key(uint32_t keycode) {
    send_keycode_event(keycode, true);
    /* Tiny delay helps some hosts; keep small to avoid lag */
    k_busy_wait(800); /* 0.8ms */
    send_keycode_event(keycode, false);
}

/* Modifier press/release (use the standard keycodes) */
static void press_mod(uint32_t mod_keycode) {
    send_keycode_event(mod_keycode, true);
}
static void release_mod(uint32_t mod_keycode) {
    send_keycode_event(mod_keycode, false);
}

static void tap_with_mod(uint32_t mod_keycode, uint32_t keycode) {
    press_mod(mod_keycode);
    k_busy_wait(300);
    tap_key(keycode);
    k_busy_wait(300);
    release_mod(mod_keycode);
}

/* -------------------------------------------------------------------------- */
/* ASCII -> keycode mapping (US layout assumption)                             */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint32_t keycode;
    bool need_shift;
} keymap_entry_t;

static bool map_ascii(char c, keymap_entry_t *out) {
    if (!out) return false;

    /* Letters */
    if (c >= 'a' && c <= 'z') {
        out->keycode = (uint32_t)(KC_A + (c - 'a'));
        out->need_shift = false;
        return true;
    }
    if (c >= 'A' && c <= 'Z') {
        out->keycode = (uint32_t)(KC_A + (c - 'A'));
        out->need_shift = true;
        return true;
    }

/*
     //Digits
    if (c >= '0' && c <= '9') {
        out->keycode = (uint32_t)(KC_0 + (c - '0'));
        out->need_shift = false;
        return true;
    }
*/
        /* Digits (HID usage order is 1..9 then 0, so handle correctly) */
    if (c >= '1' && c <= '9') {
        out->keycode = (uint32_t)(KC_1 + (c - '1')); /* KC_1..KC_9 are contiguous */
        out->need_shift = false;
        return true;
    }
    if (c == '0') {
        out->keycode = (uint32_t)KC_0;
        out->need_shift = false;
        return true;
    }
    
    /* Space / newline / tab */
    if (c == ' ') { out->keycode = KC_SPACE; out->need_shift = false; return true; }
    if (c == '\n') { out->keycode = KC_ENTER; out->need_shift = false; return true; }
    if (c == '\t') { out->keycode = KC_TAB; out->need_shift = false; return true; }

    /* Punctuation (US) */
    switch (c) {
    case '-': out->keycode = KC_MINUS; out->need_shift = false; return true;
    case '_': out->keycode = KC_MINUS; out->need_shift = true;  return true;
    case '=': out->keycode = KC_EQUAL; out->need_shift = false; return true;
    case '+': out->keycode = KC_EQUAL; out->need_shift = true;  return true;

    case '[': out->keycode = KC_LBKT;  out->need_shift = false; return true;
    case '{': out->keycode = KC_LBKT;  out->need_shift = true;  return true;
    case ']': out->keycode = KC_RBKT;  out->need_shift = false; return true;
    case '}': out->keycode = KC_RBKT;  out->need_shift = true;  return true;

    case '\\': out->keycode = KC_BSLH; out->need_shift = false; return true;
    case '|':  out->keycode = KC_BSLH; out->need_shift = true;  return true;

    case ';': out->keycode = KC_SEMI;  out->need_shift = false; return true;
    case ':': out->keycode = KC_SEMI;  out->need_shift = true;  return true;

    case '\'': out->keycode = KC_SQT;  out->need_shift = false; return true;
    case '"':  out->keycode = KC_SQT;  out->need_shift = true;  return true;

    case ',': out->keycode = KC_COMMA; out->need_shift = false; return true;
    case '<': out->keycode = KC_COMMA; out->need_shift = true;  return true;

    case '.': out->keycode = KC_DOT;   out->need_shift = false; return true;
    case '>': out->keycode = KC_DOT;   out->need_shift = true;  return true;

    case '/': out->keycode = KC_FSLH;  out->need_shift = false; return true;
    case '?': out->keycode = KC_FSLH;  out->need_shift = true;  return true;

    case '`': out->keycode = KC_GRAVE; out->need_shift = false; return true;
    case '~': out->keycode = KC_GRAVE; out->need_shift = true;  return true;

    /* US shift-number row symbols (note: assumes KC_1..KC_0 are contiguous) */
    case '!': out->keycode = KC_1; out->need_shift = true; return true;
    case '@': out->keycode = KC_2; out->need_shift = true; return true;
    case '#': out->keycode = KC_3; out->need_shift = true; return true;
    case '$': out->keycode = KC_4; out->need_shift = true; return true;
    case '%': out->keycode = KC_5; out->need_shift = true; return true;
    case '^': out->keycode = KC_6; out->need_shift = true; return true;
    case '&': out->keycode = KC_7; out->need_shift = true; return true;
    case '*': out->keycode = KC_8; out->need_shift = true; return true;
    case '(': out->keycode = KC_9; out->need_shift = true; return true;
    case ')': out->keycode = KC_0; out->need_shift = true; return true;

    default:
        break;
    }

    return false;
}

static void send_text_plain(const char *s) {
    if (!s) return;

    for (const char *p = s; *p; p++) {
        keymap_entry_t km;
        if (!map_ascii(*p, &km)) {
            /* Unknown -> ignore (or log) */
            LOG_DBG("unknown char: 0x%02x", (unsigned char)*p);
            continue;
        }

        if (km.need_shift) {
            tap_with_mod(KC_LSHIFT, km.keycode);
        } else {
            tap_key(km.keycode);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* {#...} token handling                                                      */
/* -------------------------------------------------------------------------- */

static bool parse_fkey(const char *name, uint32_t *out_kc) {
    if (!name || !out_kc) return false;
    /* name like "F13" */
    if (name[0] != 'F') return false;

    int n = 0;
    for (const char *p = name + 1; *p; p++) {
        if (*p < '0' || *p > '9') return false;
        n = n * 10 + (*p - '0');
    }
    if (n < 1 || n > 24) return false;

    /* ZMK has KC_F1..KC_F24 */
    *out_kc = (uint32_t)(KC_F1 + (n - 1));
    return true;
}

static bool handle_hash_token(const char *token) {
    /* token examples: "Left", "BackSpace", "F13" */
    if (!token || !*token) return false;

    uint32_t kc = 0;

    if (!strcmp(token, "Left")) kc = KC_LEFT;
    else if (!strcmp(token, "Right")) kc = KC_RIGHT;
    else if (!strcmp(token, "Up")) kc = KC_UP;
    else if (!strcmp(token, "Down")) kc = KC_DOWN;
    else if (!strcmp(token, "Home")) kc = KC_HOME;
    else if (!strcmp(token, "End")) kc = KC_END;
    else if (!strcmp(token, "Enter")) kc = KC_ENTER;
    else if (!strcmp(token, "Tab")) kc = KC_TAB;
    else if (!strcmp(token, "Escape")) kc = KC_ESCAPE;
    else if (!strcmp(token, "BackSpace")) kc = KC_BSPC;
    else if (!strcmp(token, "Delete")) kc = KC_DEL;
    else if (parse_fkey(token, &kc)) {
        /* ok */
    } else {
        return false;
    }

    tap_key(kc);
    return true;
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/*
 * Entry point used by core:
 * - It may receive the command strings from mejiro_tables.c (like "{#Left}")
 */
void mejiro_send_roman(const char *s) {
    if (!s || !*s) return;

    /* Special commands beginning with '=' (subset) */
    if (!strcmp(s, "=undo")) {
        tap_with_mod(KC_LCTRL, KC_Z);
        return;
    }

    /* Parse stream with {...} blocks */
    const char *p = s;
    while (*p) {
        if (*p != '{') {
            /* emit plain chunk until '{' */
            const char *start = p;
            while (*p && *p != '{') p++;

            /* copy small chunk to buffer */
            char buf[64];
            size_t len = (size_t)(p - start);
            while (len > 0) {
                size_t n = (len < sizeof(buf) - 1) ? len : (sizeof(buf) - 1);
                for (size_t i = 0; i < n; i++) buf[i] = start[i];
                buf[n] = '\0';
                send_text_plain(buf);
                start += n;
                len -= n;
            }
            continue;
        }

        /* now *p == '{' */
        const char *block = p;
        const char *end = strchr(block, '}');
        if (!end) {
            /* unmatched '{' -> rest as plain */
            send_text_plain(block);
            return;
        }

        /* content inside braces */
        size_t inner_len = (size_t)(end - (block + 1));
        char inner[64];
        if (inner_len >= sizeof(inner)) inner_len = sizeof(inner) - 1;
        for (size_t i = 0; i < inner_len; i++) inner[i] = block[1 + i];
        inner[inner_len] = '\0';

        /* Handle {^literal^} */
        if (inner[0] == '^') {
            char *caret_end = strrchr(inner, '^');
            if (caret_end && caret_end != inner) {
                /* extract between first '^' and last '^' */
                *caret_end = '\0';
                send_text_plain(inner + 1);
            } else {
                /* malformed -> treat as plain */
                send_text_plain("{");
                send_text_plain(inner);
                send_text_plain("}");
            }
            p = end + 1;
            continue;
        }

        /* Handle {#Token} */
        if (inner[0] == '#') {
            if (!handle_hash_token(inner + 1)) {
                /* unknown -> treat as plain */
                send_text_plain("{");
                send_text_plain(inner);
                send_text_plain("}");
            }
            p = end + 1;
            continue;
        }

        /* Unknown {...} -> treat as literal */
        send_text_plain("{");
        send_text_plain(inner);
        send_text_plain("}");
        p = end + 1;
    }
}

/*
 * Convenience: send a single keycode (tap)
 * If core wants to call it directly.
 */
void mejiro_tap_keycode(uint32_t keycode) {
    tap_key(keycode);
}

/* -------------------------------------------------------------------------- */
/* Linker-required symbol (called from mejiro_core.c)                          */
/* -------------------------------------------------------------------------- */

/* mejiro_core.c が呼ぶ名前に合わせるだけの薄いラッパ */
void mejiro_send_roman_exec(const char *s) {
    mejiro_send_roman(s);
    return 0;
}

/* -------------------------------------------------------------------------- */
/* Naginata hook stub (called from behavior_naginata.c)                        */
/* -------------------------------------------------------------------------- */

bool mejiro_try_emit_from_nginput(const NGListArray *nginput) {
    ARG_UNUSED(nginput);
    /* まだメジロ変換を実装していないので「処理しなかった」扱いで返す */
    return false;
}

