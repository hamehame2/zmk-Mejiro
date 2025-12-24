/*
 * SPDX-License-Identifier: MIT
 */
#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <drivers/behavior.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>

#include <dt-bindings/zmk/keys.h>    // F1..F24 etc
#include <dt-bindings/zmk/mejiro.h>  // MJ_* (param defs)

/*
 * 切り分け用:
 *  - &mejiro の binding が呼ばれていることを確認するため
 *    param1 に関係なく常に F5 を出す
 *
 * 本実装に移るときはここを「param1 → (H/T/K等の)内部処理」に差し替える。
 */
static uint32_t mj_param_to_keycode(uint32_t p) {
    ARG_UNUSED(p);
    return F5;
}

static int on_press(struct zmk_behavior_binding *binding,
                    struct zmk_behavior_binding_event event) {
    uint32_t kc = mj_param_to_keycode(binding->param1);
    if (!kc) {
        return ZMK_BEHAVIOR_OPAQUE;
    }
    return raise_zmk_keycode_state_changed_from_encoded(kc, true, event.timestamp);
}

static int on_release(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    uint32_t kc = mj_param_to_keycode(binding->param1);
    if (!kc) {
        return ZMK_BEHAVIOR_OPAQUE;
    }
    return raise_zmk_keycode_state_changed_from_encoded(kc, false, event.timestamp);
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_press,
    .binding_released = on_release,
};

/*
 * 重要:
 * - BEHAVIOR_DT_INST_DEFINE(0, ...) を直書きすると
 *   「DTインスタンスが存在しない」時にコンパイルエラーになりうる。
 * - DT_INST_FOREACH_STATUS_OKAY だけにしておけば、
 *   インスタンスが無い場合は何も生成されず安全。
 */
#define MEJIRO_INST(n)                                                                          \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                             \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MEJIRO_INST)
