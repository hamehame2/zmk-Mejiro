#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

#include "mejiro.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_mejiro_config {
    /* reserved for future stable params */
};

struct behavior_mejiro_data {
    struct mejiro_state st;

    /* snapshot of last stroke (critical to avoid "all released => 0 masks") */
    uint32_t last_left;
    uint32_t last_right;
    uint32_t last_mod;
    bool stroke_dirty;
};

static int behavior_mejiro_init(const struct device *dev) {
    struct behavior_mejiro_data *data = dev->data;
    mejiro_reset(&data->st);
    data->last_left = data->last_right = data->last_mod = 0;
    data->stroke_dirty = false;
    return 0;
}

/* Called from keymap: binding->param1 is enum mejiro_key_id */
static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = DEVICE_DT_GET(DT_DRV_INST(0));
    struct behavior_mejiro_data *data = dev->data;

    enum mejiro_key_id id = (enum mejiro_key_id)binding->param1;

    /* press: update live masks */
    mejiro_on_key_event(&data->st, id, true);

    /* keep snapshot updated while any is down */
    data->last_left  = data->st.left_mask;
    data->last_right = data->st.right_mask;
    data->last_mod   = data->st.mod_mask;
    data->stroke_dirty = true;

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    const struct device *dev = DEVICE_DT_GET(DT_DRV_INST(0));
    struct behavior_mejiro_data *data = dev->data;

    enum mejiro_key_id id = (enum mejiro_key_id)binding->param1;

    /* release: update live masks */
    mejiro_on_key_event(&data->st, id, false);

    /* if everything is released and we had a stroke snapshot, emit */
    if (!data->st.left_mask && !data->st.right_mask && !data->st.mod_mask && data->stroke_dirty) {
        const char *roman = NULL;
        if (mejiro_lookup_roman(data->last_left, data->last_right, data->last_mod, &roman)) {
            (void)mejiro_send_roman(roman);
        }
        data->stroke_dirty = false;
        data->last_left = data->last_right = data->last_mod = 0;
        data->st.active = false;
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

/*
 * SAFE device instantiation:
 * - If devicetree has no "zmk,behavior-mejiro" node, this expands to nothing (no compile error).
 * - If it exists, all instances are defined.
 */
#define MEJ_INST(inst)                                                                  \
    DEVICE_DT_INST_DEFINE(inst,                                                         \
                          behavior_mejiro_init,                                          \
                          NULL,                                                         \
                          &((struct behavior_mejiro_data){}),                            \
                          &((struct behavior_mejiro_config){}),                          \
                          POST_KERNEL,                                                   \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                           \
                          &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MEJ_INST)
