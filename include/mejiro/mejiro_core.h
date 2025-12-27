#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <zmk_naginata/nglistarray.h>
#include <zmk/behavior.h>                 /* struct zmk_behavior_binding_event */
#include "mejiro/mejiro_key_ids.h"

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
 * Returns true if something was emitted (or would be emitted).
 */
bool mejiro_try_emit(struct mejiro_state *st);

/*
 * Optional helper if you want core to own the "released" entrypoint.
 * IMPORTANT: behavior handlers pass zmk_behavior_binding_event, NOT zmk_event_t.
 */
// naginata_release() 側の呼び出しに合わせて 2 引数に統一
bool mejiro_try_emit_from_nginput(const NGListArray *nginput, int64_t timestamp);
