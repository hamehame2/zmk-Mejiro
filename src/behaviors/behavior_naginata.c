/*
 * SPDX-License-Identifier: MIT
 */
#define DT_DRV_COMPAT zmk_behavior_naginata

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>

#include <zmk_naginata/nglist.h>
#include <zmk_naginata/nglistarray.h>
#include <zmk_naginata/naginata_func.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static NGListArray nginput;
static int64_t timestamp;

static int behavior_naginata_init(const struct device *dev) {
    ARG_UNUSED(dev);
    initializeListArray(&nginput);
    timestamp = 0;
    return 0;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    /* pressでは何もしない（あなたの方針：離した時に判定） */
    timestamp = k_uptime_get();
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);

    /* event.position を NGList にして addToListArray する（宣言に合わせる） */
    NGList one;
    initializeList(&one);
    (void)addToList(&one, (uint32_t)event.position);
    (void)addToListArray(&nginput, &one);

    /* あなたの naginata_func 側の関数名に合わせる必要があるので、
       ここは「宣言がある関数」に合わせて呼ぶこと。
       もし naginata_type_from_nglistarray が存在しないなら、
       naginata_func.h の実際の公開関数名に置換する。 */
    (void)naginata_type_from_nglistarray(&nginput, timestamp);

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_naginata_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

DEVICE_DT_INST_DEFINE(0, behavior_naginata_init, NULL, NULL, NULL,
                      APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_naginata_driver_api);
