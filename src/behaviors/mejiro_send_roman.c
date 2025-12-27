#include <stddef.h>
#include <string.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/*
 * ここは “送出方式” の差し替え点。
 * - SEND_STRING 的な物を使う
 * - keycode を順にタップする
 * - macro subsystem を叩く
 * など、あなたの既存方針に合わせる。
 *
 * 今はログだけ出して成功扱いにしない（=副作用無し）。
 */
bool mejiro_send_text(const char *text) {
    if (!text) {
        return false;
    }
    if (text[0] == '\0') {
        /* empty output is valid (e.g. suppression) */
        return true;
    }
    LOG_INF("MEJIRO emit: %s", text);
    return false; /* sending not implemented yet */
}
