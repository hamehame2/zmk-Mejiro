/*
 * SPDX-License-Identifier: MIT
 *
 * behavior_mejiro.c (clean, single-file)
 *
 * - compatible: "zmk,behavior-mejiro"
 * - #binding-cells = <1>
 * - binding->param1 = enum mejiro_key_id
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include "mejiro/mejiro_core.h"
#include <zephyr/sys/util.h>   // ARG_UNUSED
/*
 * Your repo convention:
 *   zmk-Mejiro/include/mejiro/mejiro_core.h
 *   zmk-Mejiro/include/mejiro/mejiro_key_ids.h
 */
#include "mejiro/mejiro_core.h"
#include "mejiro/mejiro_key_ids.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* -------------------------------------------------------------------------- */
/* Internal state                                                             */
/* -------------------------------------------------------------------------- */

struct behavior_mejiro_config {
    /* reserved (keep for future options) */
};

static struct {
    struct mejiro_state latched;
    struct mejiro_state down;
} g;

static inline void set_active_on_press(void) {
    g.latched.active = true;
    g.down.active = true;
}

static inline bool down_any(void) {
    return (g.down.left_mask | g.down.right_mask | g.down.mod_mask) != 0;
}

static void record_key(enum mejiro_key_id id, bool pressed) {
    if (pressed) {
        set_active_on_press();

        /* latch: OR-in */
        mejiro_on_key_event(&g.latched, id, true);

        /* down: set */
        mejiro_on_key_event(&g.down, id, true);
    } else {
        /* down: clear */
        mejiro_on_key_event(&g.down, id, false);
    }
}

/* -------------------------------------------------------------------------- */
/* ZMK behavior callbacks                                                     */
/* -------------------------------------------------------------------------- */

static int behavior_mejiro_binding_pressed(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    (void)event;

    enum mejiro_key_id id = (enum mejiro_key_id)binding->param1;
    record_key(id, true);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_mejiro_binding_released(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    (void)event;

    enum mejiro_key_id id = (enum mejiro_key_id)binding->param1;
    record_key(id, false);

    /* if nothing is down anymore -> emit using latched snapshot */
    if (!down_any() && g.latched.active) {
        /*
         * IMPORTANT:
         * ここで「latched」を確定ストロークとして出力する。
         * core 側に確定出力APIを用意しているなら、必ずそれを呼ぶ。
         *
         * 例:
         *   bool ok = mejiro_try_emit(&g.latched);
         *   (void)ok;
         *
         * まだ無いなら、デバッグログだけ残して reset しておく。
         */
        LOG_INF("MEJIRO stroke completed (L=%08x R=%08x M=%08x)",
                g.latched.left_mask, g.latched.right_mask, g.latched.mod_mask);

        /* reset both */
        mejiro_reset(&g.latched);
        mejiro_reset(&g.down);
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = behavior_mejiro_binding_pressed,
    .binding_released = behavior_mejiro_binding_released,
};

/* -------------------------------------------------------------------------- */
/* Device instance                                                            */
/* -------------------------------------------------------------------------- */

#define DT_DRV_COMPAT zmk_behavior_mejiro

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define BEHAVIOR_MEJIRO_INST(n)                                                \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL,                         \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
                            &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BEHAVIOR_MEJIRO_INST)



/**
 * Naginata 側で溜めた NGListArray を見て、
 * Mejiro が処理するなら Mejiro 側で送信して true（naginata 出力を抑止）
 * まだ処理しないなら false（従来どおり naginata に出力させる）
 */
bool mejiro_try_emit_from_nginput(const NGListArray *nginput, int64_t timestamp) {
    ARG_UNUSED(timestamp);

    if (!nginput) {
        return false;
    }

    // ここは現段階では「リンクを通すための最低限」。
    // ただし「意味が無い stub」にしないため、まずは “入力が来た事実” をログで確実に掴む。
    LOG_DBG("MEJIRO hook: nginput size=%d", (int)nginput->size);

    // TODO: 次の段階で nginput をシリアライズ→テーブル参照→送信 を実装する
    return false;
}

