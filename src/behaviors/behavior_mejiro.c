/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>

#include "mejiro/mejiro_core.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_mejiro_config {
    /* (currently none) */
};

struct behavior_mejiro_data {
    /* (currently none) */
};

static int behavior_mejiro_init(const struct device *dev) {
    ARG_UNUSED(dev);
    return 0;
}

static int behavior_mejiro_binding_pressed(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);

    // Treat press as registration only; emit on release (Mejiro policy).
    // (If you later want press-side behavior, add it here.)
    return 0;
}

static int behavior_mejiro_binding_released(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);

    // Delegate to core: it should look at released-state snapshot and decide output.
    return mejiro_on_binding_released(event);
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = behavior_mejiro_binding_pressed,
    .binding_released = behavior_mejiro_binding_released,
};

/*
 * IMPORTANT:
 * Use BEHAVIOR_DT_INST_DEFINE (same style as behavior_naginata.c)
 * instead of raw DEVICE_DT_INST_DEFINE to avoid Zephyr init-level macro pitfalls.
 */
#define KP_INST(n) \
    static struct behavior_mejiro_data behavior_mejiro_data_##n; \
    static const struct behavior_mejiro_config behavior_mejiro_config_##n = {}; \
    BEHAVIOR_DT_INST_DEFINE(n, \
        behavior_mejiro_init, NULL, \
        &behavior_mejiro_data_##n, &behavior_mejiro_config_##n, \
        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
        &behavior_mejiro_driver_api)

DT_INST_FOREACH_STATUS_OKAY(KP_INST)

