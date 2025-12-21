#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static int mejiro_pressed(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event) {
    LOG_INF("MEJIRO pressed pos=%d layer=%d", event.position, event.layer);
    return 0;
}

static int mejiro_released(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    LOG_INF("MEJIRO released pos=%d layer=%d", event.position, event.layer);
    return 0;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = mejiro_pressed,
    .binding_released = mejiro_released,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL,
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_mejiro_driver_api);
