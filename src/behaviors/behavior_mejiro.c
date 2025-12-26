/*
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>

#include "mejiro_core.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* ---- data/config (今は空でOK) ---------------------------------------- */

struct behavior_mejiro_config {
    /* 予約：将来 mode 等を入れたくなったらここ */
};

struct behavior_mejiro_data {
    /* 予約：将来 state をここに置きたくなったらここ */
};

/* ---- pressed/released -------------------------------------------------- */

static int behavior_mejiro_pressed(struct zmk_behavior_binding *binding,
                                   struct zmk_behavior_binding_event event) {
    /* &mj <key_id> の <key_id> は binding->param1 に入る */
    uint8_t key_id = (uint8_t)binding->param1;
    mejiro_on_press(key_id);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_mejiro_released(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {
    uint8_t key_id = (uint8_t)binding->param1;
    mejiro_on_release(key_id);
    return ZMK_BEHAVIOR_OPAQUE;
}

/* ---- driver api -------------------------------------------------------- */

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = behavior_mejiro_pressed,
    .binding_released = behavior_mejiro_released,
};

/* ---- init -------------------------------------------------------------- */

static int behavior_mejiro_init(const struct device *dev) {
    ARG_UNUSED(dev);
    return 0;
}

/* ---- device instance --------------------------------------------------- */

DEVICE_DT_INST_DEFINE(0,
                      behavior_mejiro_init,
                      NULL,
                      NULL, /* data */
                      NULL, /* config */
                      POST_KERNEL,
                      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_mejiro_driver_api);
