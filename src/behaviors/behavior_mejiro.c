// src/behaviors/behavior_mejiro.c
#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

#include <mejiro/mejiro_core.h>
#include <mejiro/mejiro_key_ids.h>

LOG_MODULE_REGISTER(behavior_mejiro, CONFIG_ZMK_LOG_LEVEL);

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    uint32_t id = binding->param1;

    if (id >= MJ_KEY_ID_MAX) {
        LOG_WRN("mejiro: invalid key_id=%u", id);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    mejiro_core_on_press((uint8_t)id);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    uint32_t id = binding->param1;

    if (id >= MJ_KEY_ID_MAX) {
        LOG_WRN("mejiro: invalid key_id=%u", id);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    mejiro_core_on_release((uint8_t)id);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

DEVICE_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL,
                      POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                      &behavior_mejiro_driver_api);
