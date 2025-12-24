/*
 * SPDX-License-Identifier: MIT
 *
 * Minimal, compile-safe ZMK behavior driver for "zmk,behavior-mejiro".
 * Fixes:
 *  - removes duplicate behavior_mejiro_driver_api definition
 *  - does NOT hardcode instance 0 (so it won't error when no DT instance exists)
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_mejiro_config {
    /* reserved */
};

struct behavior_mejiro_data {
    /* reserved */
};

static int behavior_mejiro_binding_pressed(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return 0;
}

static int behavior_mejiro_binding_released(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return 0;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = behavior_mejiro_binding_pressed,
    .binding_released = behavior_mejiro_binding_released,
};

#define MEJIRO_INST(n)                                                                               \
    static struct behavior_mejiro_data behavior_mejiro_data_##n;                                      \
    static const struct behavior_mejiro_config behavior_mejiro_config_##n = {};                       \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, &behavior_mejiro_data_##n, &behavior_mejiro_config_##n,    \
                           POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                          \
                           &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MEJIRO_INST)
