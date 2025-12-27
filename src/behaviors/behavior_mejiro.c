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

DEVICE_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL,
                      APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_mejiro_driver_api);
