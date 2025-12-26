#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "mejiro/mejiro_key_ids.h"
#include <zmk/event_manager.h>   // zmk_event_t がここ（もしくは適切なヘッダ）
/*
 * Core state: collects keys while pressed, emits when all are released.
 */

struct mejiro_state {
    uint32_t left_mask;   /* bits for MJ_L_* */
    uint32_t right_mask;  /* bits for MJ_R_* */
    uint32_t mod_mask;    /* bits for MJ_POUND / MJ_STAR */
    bool active;
};

void mejiro_reset(struct mejiro_state *st);

/* Register a press/release of a Mejiro key-id. */
void mejiro_on_key_event(struct mejiro_state *st, enum mejiro_key_id id, bool pressed);

/*
 * Try to emit when release completes a stroke.
 * Returns true if something was emitted.
 */
bool mejiro_try_emit(struct mejiro_state *st);
int mejiro_on_binding_released(const zmk_event_t *event);
