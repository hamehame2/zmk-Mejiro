/*
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include "mejiro.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* --------------------------------------------------------------------- */
/* pressed/released handlers                                              */
/* --------------------------------------------------------------------- */

static int behavior_mejiro_binding_pressed(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    /* param1 is key_id */
    uint8_t key_id = (uint8_t)binding->param1;
    return mejiro_on_press(key_id);
}

static int behavior_mejiro_binding_released(struct zmk_behavior_binding *binding,
                                            struct zmk_behavior_binding_event event) {
    uint8_t key_id = (uint8_t)binding->param1;
    return mejiro_on_release(key_id);
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = behavior_mejiro_binding_pressed,
    .binding_released = behavior_mejiro_binding_released,
};

static int behavior_mejiro_init(const struct device *dev) {
    ARG_UNUSED(dev);
    mejiro_init();
    return 0;
}

/*
 * IMPORTANT:
 * - Use BEHAVIOR_DT_INST_DEFINE + DT_INST_FOREACH_STATUS_OKAY just like naginata.
 * - This avoids hard-failing when DT instance 0 is missing/mismatched.
 */
#define MJ_INST(n)                                                                     \
    BEHAVIOR_DT_INST_DEFINE(n,                                                         \
                            behavior_mejiro_init,                                      \
                            NULL, /* pm */                                             \
                            NULL, /* data */                                           \
                            NULL, /* config */                                         \
                            POST_KERNEL,                                               \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                       \
                            &behavior_mejiro_driver_api)

DT_INST_FOREACH_STATUS_OKAY(MJ_INST)
