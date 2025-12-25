/* -------------------------------------------------------------------------
 * FILE: src/behaviors/behavior_mejiro.c
 * ------------------------------------------------------------------------- */
/*
 * SPDX-License-Identifier: MIT
 *
 * ZMK behavior driver for "zmk,behavior-mejiro".
 *  - records key press/release of 7 logical Mejiro keys (H/TL/NL/TR/NR/KL/KR)
 *  - commits when all keys are released (in mejiro_core)
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include <dt-bindings/zmk/mejiro.h>
#include <mejiro/mejiro_core.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_mejiro_config { /* reserved */ };
struct behavior_mejiro_data   { /* reserved */ };

static int behavior_mejiro_binding_pressed(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    /* param1 is the logical key id (MJ_*) */
    const uint8_t key_id = (uint8_t)binding->param1;

    /* accept only 0..6 (H/TL/NL/TR/NR/KL/KR) */
    if (key_id > MJ_KR) {
        return 0;
    }

    mejiro_core_on_press(key_id);
    return 0;
}

static int behavior_mejiro_binding_released(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    const uint8_t key_id = (uint8_t)binding->param1;

    if (key_id > MJ_KR) {
        return 0;
    }

    mejiro_core_on_release(key_id);
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
