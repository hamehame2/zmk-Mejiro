/*
 * SPDX-License-Identifier: MIT
 *
 * behavior_mejiro.c
 * - Fix: remove duplicate behavior_mejiro_driver_api definition
 * - Debug: emit ASCII marker for each MJ_* param via mej_output_utf8()
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>
#include <dt-bindings/zmk/mejiro.h>

/* Implemented in src/behaviors/mejiro_send_roman.c */
extern void mej_output_utf8(const char *s);

struct behavior_mejiro_config {
    /* reserved */
};

struct behavior_mejiro_data {
    /* reserved */
};

static int behavior_mejiro_binding_pressed(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    /* ZMK: behavior binding parameters */
    const uint32_t p = binding ? binding->param1 : 0;

    /* Debug outputs:
     *  TR -> a
     *  KL -> b
     *  KR -> c
     *   H -> h
     *  TL -> t
     *  NL -> n
     *  NR -> r
     */
    switch (p) {
    case MJ_TR:
        mej_output_utf8("a");
        break;
    case MJ_KL:
        mej_output_utf8("b");
        break;
    case MJ_KR:
        mej_output_utf8("c");
        break;
    case MJ_H:
        mej_output_utf8("h");
        break;
    case MJ_TL:
        mej_output_utf8("t");
        break;
    case MJ_NL:
        mej_output_utf8("n");
        break;
    case MJ_NR:
        mej_output_utf8("r");
        break;
    default:
        /* unknown/unused param -> no-op */
        break;
    }

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
