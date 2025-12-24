/*
 * SPDX-License-Identifier: MIT
 */
#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <drivers/behavior.h>

#include <zmk/events/keycode_state_changed.h>
#include <zmk/event_manager.h>

#include <dt-bindings/zmk/keys.h>      // F13 など
#include <dt-bindings/zmk/mejiro.h>    // MJ_* 定義

static uint32_t mj_param_to_keycode(uint32_t p) {
    // まず「押された」ことを確実に見たいので、全部 F13 に潰す
    // （切り分けが終わったら、ここを H/T/K... に戻す）
    ARG_UNUSED(p);
    return F13;
}

static int on_press(struct zmk_behavior_binding *binding,
                    struct zmk_behavior_binding_event event) {
    uint32_t kc = mj_param_to_keycode(binding->param1);
    if (!kc) { return ZMK_BEHAVIOR_OPAQUE; }

    return raise_zmk_keycode_state_changed_from_encoded(kc, true, event.timestamp);
}

static int on_release(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    uint32_t kc = mj_param_to_keycode(binding->param1);
    if (!kc) { return ZMK_BEHAVIOR_OPAQUE; }

    return raise_zmk_keycode_state_changed_from_encoded(kc, false, event.timestamp);
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_press,
    .binding_released = on_release,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL,
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_mejiro_driver_api);
