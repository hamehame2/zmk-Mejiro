/*
 * mejiro_send_roman.c
 *
 * Minimal "UTF-8/ASCII -> key events" backend for Mejiro.
 * - Uses <dt-bindings/zmk/keys.h> keycodes only (no HID_USAGE_* arithmetic).
 * - Uses raise_zmk_keycode_state_changed_from_encoded(kc, state, timestamp) (3 args).
 *
 * Supported characters:
 *   a-z, A-Z, 0-9, space, \n
 *   . , - / ' ;
 *   ? !  (as shifted / and shifted 1)
 */

#include <zephyr/kernel.h>

#include <zmk/events/keycode_state_changed.h>
#include <dt-bindings/zmk/keys.h>

static inline int64_t now_ms(void) { return k_uptime_get(); }

static void key_event(uint32_t kc, bool pressed) {
    raise_zmk_keycode_state_changed_from_encoded(kc, pressed, now_ms());
}

static void tap_kc(uint32_t kc) {
    key_event(kc, true);
    key_event(kc, false);
}

static void tap_shifted(uint32_t kc) {
    key_event(LSHFT, true);
    tap_kc(kc);
    key_event(LSHFT, false);
}

static uint32_t letter_to_kc(char c) {
    /* dt-bindings provides A..Z keycodes */
    if (c >= 'a' && c <= 'z') return (uint32_t)(A + (c - 'a'));
    if (c >= 'A' && c <= 'Z') return (uint32_t)(A + (c - 'A'));
    return 0;
}

static uint32_t digit_to_kc(char c) {
    /* ZMK uses N1..N0 for digits */
    switch (c) {
    case '1': return N1;
    case '2': return N2;
    case '3': return N3;
    case '4': return N4;
    case '5': return N5;
    case '6': return N6;
    case '7': return N7;
    case '8': return N8;
    case '9': return N9;
    case '0': return N0;
    default:  return 0;
    }
}

static uint32_t punct_to_kc(char c, bool *need_shift) {
    *need_shift = false;

    switch (c) {
    case ' ': return SPACE;
    case '\n': return ENTER;

    case '.': return DOT;
    case ',': return COMMA;
    case '-': return MINUS;
    case '/': return SLASH;
    case '\'': return APOS;
    case ';': return SEMI;

    /* Derived punctuation */
    case '?': *need_shift = true; return SLASH; /* Shift + / */
    case '!': *need_shift = true; return N1;    /* Shift + 1 */

    default: return 0;
    }
}

static uint32_t ascii_to_kc(char c, bool *need_shift) {
    *need_shift = false;

    uint32_t kc = letter_to_kc(c);
    if (kc) {
        if (c >= 'A' && c <= 'Z') *need_shift = true;
        return kc;
    }

    kc = digit_to_kc(c);
    if (kc) return kc;

    return punct_to_kc(c, need_shift);
}

void mej_output_utf8(const char *s) {
    if (!s) return;

    for (const char *p = s; *p; p++) {
        bool shift = false;
        uint32_t kc = ascii_to_kc(*p, &shift);
        if (!kc) {
            /* unsupported char: skip */
            continue;
        }
        if (shift) tap_shifted(kc);
        else tap_kc(kc);
    }
}
