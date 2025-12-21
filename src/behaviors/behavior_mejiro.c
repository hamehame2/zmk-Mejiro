/*
 * Minimal Mejiro behavior (ZMK / Zephyr 4.x)
 * Purpose: verify that &mj bindings are actually invoked.
 */
#define DT_DRV_COMPAT zmk_behavior_mejiro
#error "MEJIRO_C_IS_COMPILING"

#include <zephyr/device.h>
#include <zephyr/sys/printk.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

static int mejiro_pressed(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    printk("MJ press: p1=%d p2=%d\n", binding->param1, binding->param2);
    return 0;
}

static int mejiro_released(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    printk("MJ release: p1=%d p2=%d\n", binding->param1, binding->param2);
    return 0;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = mejiro_pressed,
    .binding_released = mejiro_released,
};

static int behavior_mejiro_init(const struct device *dev) {
    ARG_UNUSED(dev);
    printk("MJ init\n");
    return 0;
}

#define MEJIRO_INST(n)                                                     \
    DEVICE_DT_INST_DEFINE(n,                                               \
                          behavior_mejiro_init,                            \
                          NULL,                                            \
                          NULL,                                            \
                          NULL,                                            \
                          POST_KERNEL,                                     \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,             \
                          &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MEJIRO_INST)

