/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <drivers/behavior.h>

#include <zmk/behavior.h>

#include "mejiro_core.h"

/*
 * ZMK behavior callback:
 * - must return int
 * - your mejiro_on_press/mejiro_on_release can be void; we return 0 here.
 */

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    /* param1 is used by your &mj binding (e.g. layer id / key id) */
    mejiro_on_press((uint8_t)binding->param1);
    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    mejiro_on_release((uint8_t)binding->param1);
    return 0;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

DEVICE_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, APPLICATION,
                      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_mejiro_driver_api);
