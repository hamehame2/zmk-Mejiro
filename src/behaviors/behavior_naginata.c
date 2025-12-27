/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_naginata

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>

#include <zmk_naginata/nglist.h>
#include <zmk_naginata/nglistarray.h>
#include <zmk_naginata/naginata_func.h>

/* --- Mejiro hook -------------------------------------------------------- */
/*
 * 「薙刀の入力列(NGListArray)が確定した瞬間（全離し）」で
 * Mejiro 側に “確定済み入力” を渡して出力を試す。
 *
 * 返り値 true: Mejiro が消費して出力した（薙刀の通常出力は抑止）
 * 返り値 false: Mejiro は不一致（薙刀の通常出力にフォールバック）
 */
bool mejiro_try_emit_from_nginput(const NGListArray *nginput, int64_t timestamp);

static inline void clearListArray(NGListArray *a) {
    while (a && a->size > 0) {
        removeFromListArrayAt(a, 0);
    }
}
/* ---------------------------------------------------------------------- */

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct naginata_config {
    bool tategaki;
    int os;
};

struct naginata_config naginata_config;

static uint32_t pressed_keys;
static int64_t timestamp;

extern bool ng_excluded;
extern bool ng_enabled;
extern uint32_t (*ng_mode_keymap)[2];

static int naginata_init(const struct device *dev) {
    ARG_UNUSED(dev);
    pressed_keys = 0;
    timestamp = 0;
    return 0;
}

static int naginata_listener(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (!ng_enabled) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    /* 押下状態ビット（薙刀式側のロジック） */
    if (ev->state) {
        pressed_keys |= (1U << ev->keycode);
    } else {
        pressed_keys &= ~(1U << ev->keycode);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(naginata, naginata_listener);
ZMK_SUBSCRIPTION(naginata, zmk_keycode_state_changed);

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);

    if (!ng_enabled) {
        return ZMK_BEHAVIOR_OPAQUE;
    }

    /* ここでは何もしない（薙刀式は “離した時” に確定させる） */
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);

    if (!ng_enabled) {
        return ZMK_BEHAVIOR_OPAQUE;
    }

    /* 薙刀式の “離し” 確定処理に合わせて timestamp を更新 */
    timestamp = event.timestamp;

    /* ---- ここから下は「あなたの薙刀式既存処理」に合わせる ----
     * ※ 実際の薙刀式実装では、ここで NGListArray nginput を構築して
     *    naginata_func 側に渡して変換・出力します。
     *
     * あなたの環境では既に NGListArray nginput が存在して
     * 「全離しで output」しているので、その直前に hook を挟むのが正解。
     *
     * ここでは “典型的な流れ” の形で書いています。
     */

    static NGListArray nginput;
    static bool nginput_initialized = false;

    if (!nginput_initialized) {
        initListArray(&nginput);
        nginput_initialized = true;
    }

    /* ここで nginput に押下情報を足す（あなたの元コードに合わせて実装済みのはず）
     * 例: addToListArray(&nginput, some_ng_keycode);
     *
     * このサンプルでは “既に nginput が更新されている前提” にします。
     */

    /* ---- 「全キーが離された」＝確定点 ---- */
    if (pressed_keys == 0) {
        /* 1) まず Mejiro hook を試す（Mejiro が合致したら薙刀出力は抑止） */
        if (mejiro_try_emit_from_nginput(&nginput, timestamp)) {
            clearListArray(&nginput);
            return ZMK_BEHAVIOR_OPAQUE;
        }

        /* 2) Mejiro が不一致なら、従来通り薙刀式の変換と出力 */
        naginata_type_from_nglistarray(&nginput, timestamp);

        /* 3) 片付け */
        clearListArray(&nginput);
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_naginata_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

static int behavior_naginata_init(const struct device *dev) {
    naginata_init(dev);
    return 0;
}

DEVICE_DT_INST_DEFINE(0, behavior_naginata_init, NULL, NULL, NULL, APPLICATION,
                      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_naginata_driver_api);
