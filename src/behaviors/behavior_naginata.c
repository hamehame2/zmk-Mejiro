/*
 * SPDX-License-Identifier: MIT
 */
#define DT_DRV_COMPAT zmk_behavior_naginata

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

/* zmk-naginata (このモジュール内の実在ヘッダ) */
#include <zmk_naginata/nglist.h>
#include <zmk_naginata/nglistarray.h>
#include <zmk_naginata/naginata_func.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct naginata_config {
    bool tategaki;
    int os;
};

static uint32_t pressed_keys;
static int64_t timestamp;

/* nginput は「離した時にまとめて処理」用 */
static NGListArray nginput;

extern bool ng_excluded;
extern bool ng_enabled;
extern uint32_t (*ng_mode_keymap)[2];

static int behavior_naginata_init(const struct device *dev) {
    ARG_UNUSED(dev);
    pressed_keys = 0;
    timestamp = 0;
    initListArray(&nginput);
    return 0;
}

/* ここは ZMK behavior の標準シグネチャ */
static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);

    /* naginata 側と合わせるため timestamp は event.timestamp を使う */
    timestamp = event.timestamp;

    /* “押下時”は pressed_keys に積むだけ（ZMKの既存薙刀式と同じ思想） */
    pressed_keys |= (1u << event.position);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);

    timestamp = event.timestamp;

    /* 離したので pressed_keys から落とす */
    pressed_keys &= ~(1u << event.position);

    /*
     * ここはあなたの既存薙刀式実装に合わせる：
     * - event.position を “薙刀式の入力ID” に変換して nginput に積む
     *
     * 最小 bring-up としては「position をそのまま積む」。
     * 実運用では ng_mode_keymap 等で position->keycode を変換して add しているはず。
     */
    addToListArray(&nginput, (uint16_t)event.position);

    /*
     * すべて離されたタイミングでまとめて確定（薙刀式既存と同じ）
     */
    if (pressed_keys == 0) {
        naginata_type_from_nglistarray(&nginput, timestamp);
        /* 次の入力に備えてクリア */
        while (nginput.size > 0) {
            removeFromListArrayAt(&nginput, 0);
        }
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_naginata_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

DEVICE_DT_INST_DEFINE(0,
                      behavior_naginata_init,
                      NULL,
                      NULL,
                      NULL,
                      POST_KERNEL,
                      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_naginata_driver_api);
