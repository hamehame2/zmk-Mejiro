/*
 * Minimal Mejiro behavior: just logs param on press/release.
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static int mejiro_pressed(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    /* binding->param1 が keymap の &mj <param> の param */
    LOG_INF("MEJIRO press: param=%d", binding->param1);
    return 0;
}

static int mejiro_released(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    LOG_INF("MEJIRO release: param=%d", binding->param1);
    return 0;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = mejiro_pressed,
    .binding_released = mejiro_released,
};

static int behavior_mejiro_init(const struct device *dev) {
    ARG_UNUSED(dev);
    return 0;
}

#define MEJIRO_INST(n)                                                                       \
    DEVICE_DT_INST_DEFINE(n,                                                                 \
                          behavior_mejiro_init,                                              \
                          NULL,                                                              \
                          NULL,                                                              \
                          NULL,                                                              \
                          APPLICATION,                                                       \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                               \
                          &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MEJIRO_INST)
