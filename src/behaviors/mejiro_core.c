/*
 * Minimal Mejiro core glue for ZMK.
 *
 * This file intentionally avoids any dependency on generated Mejiro tables
 * (mj_hepburn_len, etc.) so the module can compile in isolation.
 *
 * It provides the exported symbol used by the Naginata hook:
 *   bool mejiro_try_emit_from_nginput(const NGListArray *nginput);
 *
 * Extend later: parse nginput and emit romaji / kana based on your mapping.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/hid.h>

#include <zmk_naginata/nglistarray.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/*
 * Very small helper: type ASCII string by sending each byte as a HID key.
 * Note: this relies on ZMK's hid layer accepting ASCII in zmk_hid_keyboard_press().
 */
static void type_ascii(const char *s) {
    if (!s) {
        return;
    }

    for (const char *p = s; *p; p++) {
        zmk_hid_keyboard_press((uint8_t)*p);
        zmk_hid_keyboard_release((uint8_t)*p);
    }
}

bool mejiro_try_emit_from_nginput(const NGListArray *nginput) {
    ARG_UNUSED(nginput);

    /*
     * Build-safe default: do nothing.
     *
     * When you start wiring Mejiro, replace this with:
     *  - inspect nginput (strokes collected by Naginata)
     *  - decide Mejiro output
     *  - call type_ascii("...") or a kana emitter
     *  - return true when you handled the stroke
     */

    return false;
}
