#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include <dt-bindings/zmk/keys.h>

LOG_MODULE_REGISTER(behavior_mejiro, CONFIG_ZMK_LOG_LEVEL);

static int mejiro_pressed(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event)
{
    ARG_UNUSED(binding);

    /* ZMKのbindingは環境によって behavior_dev が "const char *" の場合がある。
       ここでは "kp" を名前で指定して &kp と同じ動きを呼ぶ。 */
    struct zmk_behavior_binding kp_esc = {
        .behavior_dev = "kp",
        .param1 = ESC,
        .param2 = 0,
    };

    return zmk_behavior_invoke_binding(&kp_esc, event, true);
}

static int mejiro_released(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event)
{
    ARG_UNUSED(binding);

    struct zmk_behavior_binding kp_esc = {
        .behavior_dev = "kp",
        .param1 = ESC,
        .param2 = 0,
    };

    return zmk_behavior_invoke_binding(&kp_esc, event, false);
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = mejiro_pressed,
    .binding_released = mejiro_released,
};

static int behavior_mejiro_init(const struct device *dev)
{
    ARG_UNUSED(dev);
    return 0;
}

#define MEJIRO_INST(n)                                                    \
    DEVICE_DT_INST_DEFINE(n,                                              \
                          behavior_mejiro_init,                           \
                          NULL,                                           \
                          NULL,                                           \
                          NULL,                                           \
                          POST_KERNEL,                                    \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,            \
                          &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MEJIRO_INST)
