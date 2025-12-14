/*
 * Mejiro core (minimal / debug):
 * - Reads the confirmed chord(s) in NGListArray (built by behavior_naginata)
 * - Emits visible ASCII via ZMK keycode events
 *
 * Goal: prove "pickup" (collect chord) and "send" (emit key events) are wired.
 * You can later replace the mapping section with your real Mejiro dictionary.
 */

#include <zephyr/kernel.h>
#include <zephyr/usb/class/usb_hid.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>

#include <zmk_naginata/nglist.h>
#include <zmk_naginata/nglistarray.h>
#include <zmk_naginata/naginata_func.h> /* for Q/W/E/... key symbols */

/* ---- tiny ASCII sender -------------------------------------------------- */

static void send_encoded(uint32_t encoded) {
    /* press */
    raise_zmk_keycode_state_changed_from_encoded(encoded, true, 0, k_uptime_get());
    /* release */
    raise_zmk_keycode_state_changed_from_encoded(encoded, false, 0, k_uptime_get());
}

static uint32_t hid_for_ascii(char c, bool *need_shift) {
    *need_shift = false;

    if (c >= 'a' && c <= 'z') {
        return ZMK_HID_USAGE(HID_USAGE_KEY, (HID_USAGE_KEYBOARD_A + (c - 'a')));
    }

    if (c >= 'A' && c <= 'Z') {
        *need_shift = true;
        return ZMK_HID_USAGE(HID_USAGE_KEY, (HID_USAGE_KEYBOARD_A + (c - 'A')));
    }

    if (c >= '0' && c <= '9') {
        return ZMK_HID_USAGE(HID_USAGE_KEY, (HID_USAGE_KEYBOARD_0_AND_CLOSING_PAREN + (c - '0')));
    }

    switch (c) {
    case ' ': return ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEYBOARD_SPACEBAR);
    case '\n': return ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEYBOARD_RETURN_ENTER);
    case '-': return ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEYBOARD_MINUS_AND_UNDERSCORE);
    default:
        /* unsupported -> ignore */
        return 0;
    }
}

static void send_ascii_char(char c) {
    bool shift = false;
    uint32_t kc = hid_for_ascii(c, &shift);
    if (!kc) {
        return;
    }

    if (shift) {
        uint32_t lshift = ZMK_HID_USAGE(HID_USAGE_KEY, HID_USAGE_KEYBOARD_LEFT_SHIFT);
        raise_zmk_keycode_state_changed_from_encoded(lshift, true, 0, k_uptime_get());
        send_encoded(kc);
        raise_zmk_keycode_state_changed_from_encoded(lshift, false, 0, k_uptime_get());
    } else {
        send_encoded(kc);
    }
}

static void mej_output_utf8(const char *s) {
    for (const char *p = s; p && *p; p++) {
        send_ascii_char(*p);
    }
}

/* ---- helpers ------------------------------------------------------------ */

static bool has_key(const NGList *keys, uint32_t kc) {
    for (int i = 0; i < keys->size; i++) {
        if (keys->elements[i] == kc) {
            return true;
        }
    }
    return false;
}

/*
 * Called from behavior_naginata when a chord is confirmed.
 * Return true if we emitted something (so behavior_naginata should NOT run ng_type()).
 */
bool mejiro_try_emit_from_nginput(const NGListArray *nginput) {
    if (!nginput || nginput->size <= 0) {
        return false;
    }

    const NGList *chord = &nginput->elements[0];

    /* Debug mapping: output a short tag that depends on which key was in chord */
    if (has_key(chord, Q)) {
        mej_output_utf8("mq");
        return true;
    }
    if (has_key(chord, W)) {
        mej_output_utf8("mw");
        return true;
    }
    if (has_key(chord, E)) {
        mej_output_utf8("me");
        return true;
    }

    /* default */
    mej_output_utf8("m?");
    return true;
}
