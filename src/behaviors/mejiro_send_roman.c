#include <mejiro/mejiro_send_roman.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mejiro_send_roman, CONFIG_ZMK_LOG_LEVEL);

/*
 * Minimal stub:
 * You will likely replace this with proper keycode emission.
 * For now, return false if roman is NULL/empty.
 */
bool mejiro_send_roman(const char *roman) {
    if (!roman || !roman[0]) {
        return false;
    }

    /* TODO: implement actual key output (SEND_STRING equivalent for ZMK) */
    LOG_DBG("mejiro_send_roman: '%s' (stub)", roman);
    return true;
}
