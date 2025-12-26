/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/keymap.h>

#include "mejiro_core.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/*
 * この behavior は「&mj <param1>」の param1 を受け取り、
 * mejiro_core 側へ press/release を通知するだけの薄いラッパです。
 *
 * 重要:
 * - ZMK の behavior callback は int を返す必要がある
 * - mejiro_on_press()/mejiro_on_release() が void の場合でも、ここは 0 を返す
 */

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    /* param1 を Mejiro のキーIDとして渡す想定 */
    const uint8_t key_id = (uint8_t)binding->param1;

    /* mejiro_on_press が void でも OK（戻り値を返そうとしない） */
    mejiro_on_press(key_id);
    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    const uint8_t key_id = (uint8_t)binding->param1;

    mejiro_on_release(key_id);
    return 0;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

static int behavior_mejiro_init(const struct device *dev) {
    ARG_UNUSED(dev);
    /* 必要ならここで初期化（現状は不要） */
    return 0;
}

/*
 * Zephyr device 定義
 * - data/config が不要なら NULL でよい
 * - init level は APPLICATION でOK
 */
DEVICE_DT_INST_DEFINE(0,
                      behavior_mejiro_init,
                      NULL,
                      NULL,
                      NULL,
                      APPLICATION,
                      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_mejiro_driver_api);
