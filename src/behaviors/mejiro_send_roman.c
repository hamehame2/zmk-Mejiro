/*
 * mejiro_send_roman_fixed.c
 *
 * Goal: provide a **known-good** "send UTF-8-as-roman/ASCII" backend for Mejiro.
 * This file avoids invalid arithmetic on encoded keycodes (A + n etc).
 *
 * It emits keycode_state_changed events using encoded keycodes from
 * <dt-bindings/zmk/keys.h>.
 *
 * Supported characters (for now):
 *   - a-z, A-Z
 *   - 0-9
 *   - space, \n
 *   - basic punctuation: . , - / ' ;
 * Extend the table as needed.
 */

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

#include <dt-bindings/zmk/keys.h>

#ifndef CONFIG_ZMK_LOG_LEVEL
#define CONFIG_ZMK_LOG_LEVEL 0
#endif

/* ---- small helpers ---- */

static inline int64_t now_ms(void) { return k_uptime_get(); }

static inline void emit_kc(uint32_t kc, bool pressed) {
    /* This is the important bit: feed the normal ZMK pipeline. */
    raise_zmk_keycode_state_changed_from_encoded(kc, pressed, now_ms());
}

static inline void tap_kc(uint32_t kc) {
    emit_kc(kc, true);
    emit_kc(kc, false);
}

static inline void tap_shifted(uint32_t kc) {
    emit_kc(LSHIFT, true);
    tap_kc(kc);
    emit_kc(LSHIFT, false);
}

/* ---- mapping tables (NO arithmetic on encoded keycodes) ---- */

static const uint32_t kc_alpha[26] = {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
};

static const uint32_t kc_digit[10] = { N0, N1, N2, N3, N4, N5, N6, N7, N8, N9 };

/* Returns 0 if unsupported. Sets *use_shift when needed. */
static uint32_t ascii_to_kc(char c, bool *use_shift) {
    *use_shift = false;

    if (c >= 'a' && c <= 'z') {
        return kc_alpha[c - 'a'];
    }
    if (c >= 'A' && c <= 'Z') {
        *use_shift = true;
        return kc_alpha[c - 'A'];
    }
    if (c >= '0' && c <= '9') {
        return kc_digit[c - '0'];
    }

    switch (c) {
        case ' ': return SPACE;
        case '\n': return ENTER;

        case '.': return DOT;
        case ',': return COMMA;
        case '-': return MINUS;
        case '/': return SLASH;
        case '\'': return SQT;
        case ';': return SEMI;

        /* Add more as needed */
        default: return 0;
    }
}

/*
 * Public API used by mejiro_core*.c
 * NOTE: This sends 1 key at a time, so HID 6KRO is NOT a limiter for long strings.
 */
void mej_output_utf8(const char *s) {
    if (!s) return;

    for (const char *p = s; *p; p++) {
        bool shift = false;
        uint32_t kc = ascii_to_kc(*p, &shift);
        if (!kc) {
            /* Unsupported char -> skip silently (or map later) */
            continue;
        }
        if (shift) tap_shifted(kc);
        else tap_kc(kc);
    }
}
