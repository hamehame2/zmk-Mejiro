#include "mejiro_send_roman.h"

#include <zmk/hid.h>
#include <zmk/keycode.h>

/*
 * Minimal stub:
 * You will likely replace this with proper keycode emission.
 * For now, return false if roman is NULL/empty.
 */
bool mejiro_send_roman(const char *roman) {
    if (!roman || !roman[0]) return false;

    /* TODO: implement actual key output (SEND_STRING equivalent for ZMK) */
    return true;
}
