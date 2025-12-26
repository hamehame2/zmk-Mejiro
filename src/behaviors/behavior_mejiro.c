/*
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

#include "mejiro_core.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* dt-binding から読む設定が必要ならここに config/data を足す */
struct behavior_mejiro_config {
    /* reserved */
};

struct behavior_mejiro_data {
    /* reserved */
};

static int on_binding_pressed(struct zmk_behavior_binding *binding,
                             struct zmk_behavior_binding_event event) {
    /* ここで param1 を “Mejiro key_id” として core に渡す */
    return mejiro_core_on_press(binding, event);
}

static int on_binding_released(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    return mejiro_core_on_release(binding, event);
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_binding_pressed,
    .binding_released = on_binding_released,
};

static int behavior_mejiro_init(const struct device *dev) {
    ARG_UNUSED(dev);
    return 0;
}

/*
 * ここが今回のクラッシュ原因になっていた場所。
 * level は “APPLICATION/POST_KERNEL/PRE_KERNEL_1/2” みたいな識別子が必須。
 * 数値や変なマクロを渡すと token paste が壊れて今回のエラーになる。
 */
DEVICE_DT_INST_DEFINE(0,
                      behavior_mejiro_init,
                      NULL,
                      NULL,
                      NULL,
                      APPLICATION,
                      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_mejiro_driver_api);
