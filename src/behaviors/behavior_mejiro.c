#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/keymap.h>

/* これを追加：mejiro_on_press/release の宣言が入っている */
#include <mejiro/mejiro_core.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    /* param1 を ID としてコアへ渡す */
    return mejiro_on_press((uint8_t)binding->param1);
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return mejiro_on_release((uint8_t)binding->param1);
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define MEJIRO_INST(n)                                                                             \
    static int behavior_mejiro_init_##n(const struct device *dev) { return 0; }                    \
    DEVICE_DT_INST_DEFINE(n, behavior_mejiro_init_##n, NULL, NULL, NULL, APPLICATION,              \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MEJIRO_INST)
