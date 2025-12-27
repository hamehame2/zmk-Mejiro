/*
 * SPDX-License-Identifier: MIT
 *
 * Actually send ASCII text by emitting ZMK keycode events.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

/* keycode symbols like A, B, N1, SPACE, ENTER, COMMA ... */
#include <dt-bindings/zmk/keys.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static void tap_key(uint32_t keycode) {
    int64_t ts = k_uptime_get();
    raise_zmk_keycode_state_changed_from_encoded(keycode, true, ts);
    raise_zmk_keycode_state_changed_from_encoded(keycode, false, ts + 1);
}

static void tap_shifted(uint32_t keycode) {
    int64_t ts = k_uptime_get();
    raise_zmk_keycode_state_changed_from_encoded(LSHFT, true, ts);
    raise_zmk_keycode_state_changed_from_encoded(keycode, true, ts);
    raise_zmk_keycode_state_changed_from_encoded(keycode, false, ts + 1);
    raise_zmk_keycode_state_changed_from_encoded(LSHFT, false, ts + 2);
}

/* Minimal ASCII -> keycode mapping for bring-up */
static bool send_ascii_char(char c) {
    /* whitespace */
    if (c == ' ') { tap_key(SPACE); return true; }
    if (c == '\n') { tap_key(ENTER); return true; }

    /* letters */
    if (c >= 'a' && c <= 'z') {
        /* dt-bindings uses A..Z symbols; use A + (c-'a') */
        uint32_t kc = (uint32_t)(A + (c - 'a'));
        tap_key(kc);
        return true;
    }
    if (c >= 'A' && c <= 'Z') {
        uint32_t kc = (uint32_t)(A + (c - 'A'));
        tap_shifted(kc);
        return true;
    }

    /* digits */
    if (c >= '1' && c <= '9') {
        uint32_t kc = (uint32_t)(N1 + (c - '1'));
        tap_key(kc);
        return true;
    }
    if (c == '0') { tap_key(N0); return true; }

    /* punctuation (minimal set) */
    switch (c) {
        case ',': tap_key(COMMA); return true;
        case '.': tap_key(DOT); return true;
        case '/': tap_key(FSLH); return true;
        case '-': tap_key(MINUS); return true;
        case ';': tap_key(SEMI); return true;
        case '\'': tap_key(SQT); return true;
        case '[': tap_key(LBKT); return true;
        case ']': tap_key(RBKT); return true;
        case '\\': tap_key(BSLH); return true;

        /* shifted punctuation common on US layout */
        case ':': tap_shifted(SEMI); return true;
        case '"': tap_shifted(SQT); return true;
        case '{': tap_shifted(LBKT); return true;
        case '}': tap_shifted(RBKT); return true;
        case '|': tap_shifted(BSLH); return true;
        case '_': tap_shifted(MINUS); return true;
        case '<': tap_shifted(COMMA); return true;
        case '>': tap_shifted(DOT); return true;
        case '?': tap_shifted(FSLH); return true;

        default:
            return false;
    }
}

bool mejiro_send_text(const char *text) {
    if (!text) {
        return false;
    }
    if (text[0] == '\0') {
        /* empty output is valid (suppression) */
        return true;
    }

    LOG_DBG("MEJIRO send_text: %s", text);

    bool all_ok = true;
    for (const char *p = text; *p; p++) {
        if (!send_ascii_char(*p)) {
            /* unknown char: ignore but mark failure */
            all_ok = false;
        }
    }
    return all_ok;
}
