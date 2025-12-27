/*
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

/*
 * ここはあなたの repo 構造に合わせている（mejiro/ 配下）
 * include/ に置いている前提：
 *   include/mejiro/mejiro_core.h
 *   include/mejiro/mejiro_key_ids.h
 */
#include "mejiro/mejiro_core.h"
#include "mejiro/mejiro_key_ids.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/*
 * This behavior expects:
 *   binding->param1 = enum mejiro_key_id
 *
 * keymap examples:
 *   &mj MJ_L_S
 *   &mj MJ_R_k
 *   &mj MJ_POUND
 */
struct behavior_mejiro_config {};

/*
 * We keep two states:
 * - latched: keys that participated in the stroke (accumulated while any key is down)
 * - down:    current pressed state
 *
 * On release:
 *   clear from down;
 *   if down becomes empty => emit using latched snapshot then reset both.
 */
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
         * ここで「latched」の内容を確定ストロークとして出力する。
         * 現状はあなたの core 側の設計（テーブル参照でemitする等）に寄せるため、
         * core の "emit" 入口がある前提で呼ぶのが正解。
         *
         * もし core に emit 関数が既にあるなら、ここをそれに置換してOK。
         * 例）mejiro_try_emit(&g.latched); など
         */

        LOG_INF("MEJIRO stroke completed (latched: L=%08x R=%08x M=%08x)",
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

DEVICE_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL,
                      APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_mejiro_driver_api);
