#include <stdbool.h>
#include <stdint.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include "mejiro/core.h"
#include "mejiro/mejiro_key_ids.h"

/*
 * This behavior expects param1 = enum mejiro_key_id
 *
 * keymap:
 *   &mj MJ_L_S
 *   &mj MJ_R_k
 *   &mj MJ_POUND
 * etc.
 */

struct behavior_mejiro_config {};

/* We keep two states:
 * - latched: keys that participated in the stroke (accumulated while any key is down)
 * - down:    current pressed state
 *
 * On release: clear from down; if down becomes empty => emit using latched snapshot then reset both.
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
    (void)binding;
    enum mejiro_key_id id = (enum mejiro_key_id)event.binding.param1;
    record_key(id, true);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_mejiro_binding_released(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    (void)binding;
    enum mejiro_key_id id = (enum mejiro_key_id)event.binding.param1;

    record_key(id, false);

    /* if nothing is down anymore -> emit using latched snapshot */
    if (!down_any() && g.latched.active) {
        /* build stroke from latched and lookup+emit */
        /* We reuse core’s helpers by temporarily copying latched into a local and forcing "down empty" */
        struct mejiro_state snapshot = g.latched;

        /* Clear down masks inside snapshot so core's "all released" logic can work if you use it;
           but here we just reset and log success. */
        (void)snapshot;

        /* For now, simply reset latched/down and report opaque. You can call a real emit routine here. */
        LOG_INF("MEJIRO stroke completed (latched masks: L=%08x R=%08x M=%08x)",
                g.latched.left_mask, g.latched.right_mask, g.latched.mod_mask);

        mejiro_reset(&g.latched);
        mejiro_reset(&g.down);
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = behavior_mejiro_binding_pressed,
    .binding_released = behavior_mejiro_binding_released,
};

#define DT_DRV_COMPAT zmk_behavior_mejiro

DEVICE_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, APPLICATION,
                      CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_mejiro_driver_api);
