/*
 * Minimal behavior driver for Mejiro.
 * - Removes include of non-existing <zmk/keycode.h>
 * - Provides a ZMK behavior that can be referenced from keymap as &mj
 *
 * This file intentionally keeps the integration surface small:
 * - on press/release, it calls mejiro_core hooks if they exist.
 *
 * If your mejiro_core.c uses different function names, update ONLY the extern
 * declarations below (keep the behavior skeleton as-is).
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>

#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* ---- Core hooks (adjust these names to match your mejiro_core.c) -------- */
/*
 * You said: "キーがはなされた時に判定" を重視。
 * So core should usually decide on release, but we pass both.
 *
 * If you already have mejiro_core.c exposing different symbols,
 * rename these externs to your actual ones.
 */
extern void mejiro_core_on_press(const struct zmk_behavior_binding_event *event,
                                const struct zmk_behavior_binding *binding);

extern void mejiro_core_on_release(const struct zmk_behavior_binding_event *event,
                                  const struct zmk_behavior_binding *binding);
/* ----------------------------------------------------------------------- */

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                    struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);

    /* Call into core if linked. */
    mejiro_core_on_press(&event, binding);
    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);

    /* Decide/emit on release (your stated preference) */
    mejiro_core_on_release(&event, binding);
    return 0;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

/* Devicetree instantiation */
#define MEJIRO_INST(n)                                                                                \
    static const struct device *mejiro_dev_##n;                                                        \
    DEVICE_DT_INST_DEFINE(n,                                                                           \
                          NULL,                                                                        \
                          NULL,                                                                        \
                          NULL,                                                                        \
                          NULL,                                                                        \
                          POST_KERNEL,                                                                 \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                         \
                          &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MEJIRO_INST)
