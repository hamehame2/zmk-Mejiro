/*
 * SPDX-License-Identifier: MIT
 *
 * ZMK behavior: zmk,behavior-mejiro
 * - compile-safe
 * - no duplicate behavior_driver_api definition
 * - DT_INST_FOREACH_STATUS_OKAY() so it won't hardcode instance 0
 *
 * Behavior semantics:
 *   binding->param1 is treated as an *encoded* ZMK keycode (same as &kp),
 *   and we raise keycode_state_changed on press/release.
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>
#include <zmk/events/keycode_state_changed.h>

static int on_mejiro_binding_pressed(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp);
}

static int on_mejiro_binding_released(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp);
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_mejiro_binding_pressed,
    .binding_released = on_mejiro_binding_released,
};

#define MEJIRO_INST(n)                                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                               \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MEJIRO_INST)
