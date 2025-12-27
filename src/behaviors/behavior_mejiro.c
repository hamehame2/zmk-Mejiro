/*
 * SPDX-License-Identifier: MIT
 *
 * behavior_mejiro.c (clean)
 *
 * compatible: "zmk,behavior-mejiro"
 * #binding-cells = <1>
 * param1 = enum mejiro_key_id
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>   // ARG_UNUSED

#include <drivers/behavior.h>

#include <zmk/behavior.h>

/* あなたのリポジトリに「実在する」ヘッダだけ使う */
#include "mejiro/mejiro_core.h"
#include "mejiro/mejiro_key_ids.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* -------------------------------------------------------------------------- */
/* Internal state                                                             */
/* -------------------------------------------------------------------------- */

static struct {
    struct mejiro_state latched; /* 押された履歴（ORで溜める） */
    struct mejiro_state down;    /* 現在押下中（押したらset/離したらclear） */
} g;

static inline void set_active_on_press(void) {
    g.latched.active = true;
    g.down.active = true;
}

static inline bool down_any(void) {
    return (g.down.left_mask | g.down.right_mask | g.down.mod_mask) != 0U;
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

    /* 何も押されていない＝ストローク確定 */
    if (!down_any() && g.latched.active) {
        /* ここが「保管→動的決定→送信」の入口 */
        LOG_DBG("MEJIRO stroke completed (L=%08x R=%08x M=%08x)",
                (unsigned)g.latched.left_mask,
                (unsigned)g.latched.right_mask,
                (unsigned)g.latched.mod_mask);

        /* TODO: 次の段階でここを
         *   mejiro_try_emit(&g.latched);
         * みたいな“確定出力API”に差し替える
         */

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
