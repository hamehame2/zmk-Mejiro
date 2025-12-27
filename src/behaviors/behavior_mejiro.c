/*
 * SPDX-License-Identifier: MIT
 *
 * behavior_mejiro.c
 *
 * - compatible: "zmk,behavior-mejiro"
 * - #binding-cells = <1>
 * - binding->param1 = enum mejiro_key_id
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>

#include <mejiro/mejiro_core.h>
#include <mejiro/mejiro_key_ids.h>

/* For the naginata hook signature */
#include <zmk_naginata/nglistarray.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* -------------------------------------------------------------------------- */
/* Internal state                                                             */
/* -------------------------------------------------------------------------- */

static struct {
    struct mejiro_state latched; /* release-when-all-up snapshot */
    struct mejiro_state down;    /* current down state */
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
    ARG_UNUSED(event);

    enum mejiro_key_id id = (enum mejiro_key_id)binding->param1;
    record_key(id, true);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_mejiro_binding_released(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    enum mejiro_key_id id = (enum mejiro_key_id)binding->param1;
    record_key(id, false);

    /* if nothing is down anymore -> emit using latched snapshot */
    if (!down_any() && g.latched.active) {
        bool ok = mejiro_try_emit(&g.latched);

        if (!ok) {
            LOG_WRN("MEJIRO emit failed (no mapping or send failed)");
        }

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

#define BEHAVIOR_MEJIRO_INST(n)                                                \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL,                         \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
                            &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BEHAVIOR_MEJIRO_INST)

/* -------------------------------------------------------------------------- */
/* Naginata hook implementation                                               */
/* -------------------------------------------------------------------------- */

/**
 * Naginata 側で溜めた NGListArray を見て、
 * Mejiro が処理するなら Mejiro 側で送信して true（naginata 出力を抑止）
 * まだ処理しないなら false（従来どおり naginata に出力させる）
 *
 * 現段階の bring-up では “必ず false” で OK（＝naginata に影響を与えない）。
 * ただし「リンク確認」と「呼ばれている事実」だけログで掴めるようにする。
 */
bool mejiro_try_emit_from_nginput(const NGListArray *nginput, int64_t timestamp) {
    ARG_UNUSED(timestamp);

    if (!nginput) {
        return false;
    }

    LOG_DBG("MEJIRO hook called: nginput size=%d", (int)nginput->size);

    /* TODO: ここで nginput を mejiro 入力に変換して emit する */
    return false;
}
