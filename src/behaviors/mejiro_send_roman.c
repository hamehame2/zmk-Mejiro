/*
 * Minimal, build-safe roman sender for ZMK 4.1.0.
 * - Fixes conflicting type error by splitting exec() and exec_cmd().
 * - Avoids returning value from void function.
 *
 * Replace the TODO section with your actual "send string" implementation.
 */

#include "mejiro_send_roman.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mejiro_send_roman, CONFIG_ZMK_LOG_LEVEL);

static inline void send_ascii_best_effort(const char *s) {
    if (!s || !*s) {
        return;
    }

    /*
     * TODO:
     * Put your actual sending implementation here.
     *
     * In many ZMK forks/modules, people implement one of:
     * - a dedicated "send string" helper
     * - invoking ZMK macro behavior programmatically
     * - using HID report helpers
     *
     * Since your tree is custom (ZaruBall + rgbled widget + ZMK Studio snippet),
     * I cannot assume the exact API name safely without your repo contents.
     *
     * For now, we only log (so build passes and you can proceed).
     */
    LOG_DBG("mejiro_send_roman_exec: '%s' (NOT SENT: TODO hook)", s);
}

void mejiro_send_roman_exec(const char *s) {
    send_ascii_best_effort(s);
    /* NOTE: do not "return 0;" here; this is void. */
}

bool mejiro_send_roman_exec_cmd(const char *cmd) {
    if (!cmd || !*cmd) {
        return false;
    }

    /*
     * If you had command parsing previously (e.g., "BS", "ENTER", "SPACE", etc),
     * implement it here and return true when handled.
     *
     * For now, treat it as a raw string to send.
     */
    send_ascii_best_effort(cmd);
    return true;
}
