/*
 * SPDX-License-Identifier: MIT
 *
 * behavior_mejiro.c
 *
 * - compatible: "zmk,behavior-mejiro"
 * - #binding-cells = <1>
 * - binding->param1 = enum mejiro_key_id (dt-binding で数値になること)
 *
 * このファイルの責務:
 *  1) &mj の press/release を受ける
 *  2) 離された瞬間に「latched」を確定ストロークとして core に渡して送信
 *  3) naginata 側から呼ばれる hook(mejiro_try_emit_from_nginput) を提供（リンク切れ防止＋将来拡張）
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h> // ARG_UNUSED

#include <drivers/behavior.h>
#include <zmk/behavior.h>

/* --- Mejiro public headers (あなたの規約: include/mejiro/...) ------------- */
#include "mejiro/mejiro_core.h"
#include "mejiro/mejiro_key_ids.h"

/* --- For naginata hook signature ----------------------------------------- */
/* NGListArray は zmk-naginata module 側の型。hook で参照するので include する */
#include <zmk_naginata/nglistarray.h>
/*
 * SPDX-License-Identifier: MIT
 *
 * ZMK behavior: mejiro
 */
#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

#include "mejiro/mejiro_core.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct mejiro_runtime {
    struct mejiro_state current;
    struct mejiro_state latched;
};

static struct mejiro_runtime g;

/* 押下/離上イベントを state に記録する */
static void record_key(uint32_t id, bool pressed) {
    /* ここで implicit declaration になってたので core側で宣言/定義済み */
    mejiro_on_key_event(&g.latched, id, pressed);
}

/* ---- ZMK behavior hooks ---- */

static int behavior_mejiro_init(const struct device *dev) {
    ARG_UNUSED(dev);
    mejiro_state_reset(&g.current);
    mejiro_state_reset(&g.latched);
    return 0;
}

static int behavior_mejiro_binding_pressed(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);

    record_key(event.position, true);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_mejiro_binding_released(struct zmk_behavior_binding *binding,
                                            struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);

    record_key(event.position, false);

    /* ここがログの致命点：mejiro_try_emit は timestamp 引数が必要 */
    int64_t ts = k_uptime_get();
    (void)mejiro_try_emit(&g.latched, ts);

    return ZMK_BEHAVIOR_OPAQUE;
}

/* ---- driver API ---- */

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = behavior_mejiro_binding_pressed,
    .binding_released = behavior_mejiro_binding_released,
};

DEVICE_DT_INST_DEFINE(0, behavior_mejiro_init, NULL, NULL, NULL,
                      APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_mejiro_driver_api);
