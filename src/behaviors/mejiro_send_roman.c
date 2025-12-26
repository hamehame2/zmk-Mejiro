/*
 * Minimal, build-safe roman sender for ZMK 4.1.0.
 * Replace TODO with your actual "send string" implementation later.
 */

#include "mejiro/mejiro_send_roman.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mejiro_send_roman, CONFIG_ZMK_LOG_LEVEL);

static inline void send_ascii_best_effort(const char *s) {
    if (!s || !*s) {
        return;
    }

    /*
     * TODO:
     * ここで「文字列を実際に送る」実装に差し替える。
     * （今はビルドを通す目的でログだけ）
     */
    LOG_DBG("mejiro_send_roman_exec: '%s' (NOT SENT: TODO hook)", s);
}

void mejiro_send_roman_exec(const char *s) { send_ascii_best_effort(s); }

bool mejiro_send_roman_exec_cmd(const char *cmd) {
    if (!cmd || !*cmd) {
        return false;
    }

    /* TODO: コマンド解釈（ENTER/BS等）を入れるならここ */
    send_ascii_best_effort(cmd);
    return true;
}
