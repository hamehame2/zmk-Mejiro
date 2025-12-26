/*
 * Minimal ZMK behavior wrapper for Mejiro chords.
 * param1: key_id (dt-bindings/zmk/mejiro.h)
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/keycode.h>

#include "mejiro/mejiro_core.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static int behavior_mejiro_press(struct zmk_behavior_binding *binding,
                                 struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    uint8_t id = (uint8_t)binding->param1;
    mejiro_on_press(id);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_mejiro_release(struct zmk_behavior_binding *binding,
                                   struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    uint8_t id = (uint8_t)binding->param1;
    mejiro_on_release(id);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = behavior_mejiro_press,
    .binding_released = behavior_mejiro_release,
};

DEVICE_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL,
                      APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_mejiro_driver_api);
