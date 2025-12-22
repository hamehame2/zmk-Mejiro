/*
 * Debug Mejiro behavior:
 *   &mj MJ_H  -> outputs 'h'
 *   &mj MJ_TL -> outputs 't'
 *   ... etc
 *
 * This is only for "is &mj alive + param1 passed?" verification.
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/mejiro.h>

LOG_MODULE_REGISTER(behavior_mejiro, CONFIG_ZMK_LOG_LEVEL);

/* Convert MJ_* logical key (binding->param1) to a visible keycode. */
static uint32_t mj_param_to_keycode(uint32_t p) {
    switch (p) {
    case MJ_H:
        return H; /* visible */
    case MJ_TL:
        return T;
    case MJ_TR:
        return R;
    case MJ_NL:
        return N;
    case MJ_NR:
        return M; /* make left/right different */
    case MJ_KL:
        return K;
    case MJ_KR:
        return J; /* make left/right different */
#ifdef MJ_X
    case MJ_X:
        return X;
#endif
    default:
        return Z; /* unknown -> Z */
    }
}

static int mejiro_pressed(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event) {
    const uint32_t kc = mj_param_to_keycode(binding ? binding->param1 : 0);

    LOG_DBG("mj pressed param=%u -> kc=%u", binding ? binding->param1 : 0, kc);

    /* Call &kp <kc> */
    struct zmk_behavior_binding kp = {
        .behavior_dev = "kp",
        .param1 = kc,
        .param2 = 0,
    };

    return zmk_behavior_invoke_binding(&kp, event, true);
}

static int mejiro_released(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    const uint32_t kc = mj_param_to_keycode(binding ? binding->param1 : 0);

    LOG_DBG("mj released param=%u -> kc=%u", binding ? binding->param1 : 0, kc);

    struct zmk_behavior_binding kp = {
        .behavior_dev = "kp",
        .param1 = kc,
        .param2 = 0,
    };

    return zmk_behavior_invoke_binding(&kp, event, false);
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = mejiro_pressed,
    .binding_released = mejiro_released,
};

static int behavior_mejiro_init(const struct device *dev) {
    ARG_UNUSED(dev);
    return 0;
}

#define MEJIRO_INST(n)                                                                          \
    DEVICE_DT_INST_DEFINE(n, behavior_mejiro_init, NULL, NULL, NULL, POST_KERNEL,               \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MEJIRO_INST)
