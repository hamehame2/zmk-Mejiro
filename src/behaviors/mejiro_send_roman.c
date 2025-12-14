/*
 * mejiro_send_roman.c
 *
 * Implements "B": send Mejiro output as sequential key events (ASCII/romaji).
 * This overrides the weak mej_output_utf8() in mejiro_core.c.
 *
 * - No 6KRO issue: we press+release one key at a time.
 * - Supports: a-z, A-Z, 0-9, space, apostrophe, hyphen, comma, dot, slash.
 * - Extend ascii_to_keycode() as needed.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <dt-bindings/zmk/keys.h>
#include <zmk/events/keycode_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static inline void send_down_up(uint32_t kc) {
    int64_t ts = k_uptime_get();
    raise_zmk_keycode_state_changed_from_encoded(kc, true, ts);
    raise_zmk_keycode_state_changed_from_encoded(kc, false, ts);
}

static inline void send_shifted(uint32_t kc) {
    int64_t ts = k_uptime_get();
    raise_zmk_keycode_state_changed_from_encoded(LSHIFT, true, ts);
    raise_zmk_keycode_state_changed_from_encoded(kc, true, ts);
    raise_zmk_keycode_state_changed_from_encoded(kc, false, ts);
    raise_zmk_keycode_state_changed_from_encoded(LSHIFT, false, ts);
}

static uint32_t ascii_to_keycode(char c, bool *needs_shift) {
    *needs_shift = false;

    if (c >= 'a' && c <= 'z') return A + (c - 'a');
    if (c >= '0' && c <= '9') return N0 + (c - '0');

    if (c >= 'A' && c <= 'Z') { *needs_shift = true; return A + (c - 'A'); }

    switch (c) {
        case ' ': return SPACE;
        case '-': return MINUS;
        case '\'': return APOSTROPHE;
        case ',': return COMMA;
        case '.': return DOT;
        case '/': return SLASH;
        default:
            return 0;
    }
}

/* Override weak symbol from mejiro_core.c */
void mej_output_utf8(const char *s) {
    if (!s || !s[0]) return;

    for (const char *p = s; *p; p++) {
        bool sh = false;
        uint32_t kc = ascii_to_keycode(*p, &sh);
        if (!kc) {
            LOG_WRN("Mejiro: unsupported char 0x%02x '%c' in output", (unsigned char)*p, *p);
            continue;
        }
        if (sh) send_shifted(kc);
        else    send_down_up(kc);
    }
}
