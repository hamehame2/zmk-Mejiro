#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <dt-bindings/zmk/keys.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* デバッグ用：押したら A を出す */
static const struct zmk_behavior_binding kp_a = {
    .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kp)), /* ★ここが重要：NAME */
    .param1 = ZMK_KEY_A,
    .param2 = 0,
};

static int mejiro_pressed(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event) {
    LOG_INF("&mj pressed pos=%d ts=%lld", event.position, event.timestamp);
    return zmk_behavior_invoke_binding(&kp_a, event, true);
}

static int mejiro_released(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    LOG_INF("&mj released pos=%d ts=%lld", event.position, event.timestamp);
    return zmk_behavior_invoke_binding(&kp_a, event, false);
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = (behavior_keymap_binding_callback_t)mejiro_pressed,
    .binding_released = (behavior_keymap_binding_callback_t)mejiro_released,
};

/* binding-cells は後述。ここは通常の定義 */
BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL,
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_mejiro_driver_api);
