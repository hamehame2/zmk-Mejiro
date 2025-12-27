/*
 * SPDX-License-Identifier: MIT
 *
 * behavior_mejiro.c
 *
 * - compatible: "zmk,behavior-mejiro"
 * - #binding-cells = <1>
 * - binding->param1 = enum mejiro_key_id (dt-binding で数値になること)
 *
 * このファイルの責務:
 *  1) &mj の press/release を受ける
 *  2) 離された瞬間に「latched」を確定ストロークとして core に渡して送信
 *  3) naginata 側から呼ばれる hook(mejiro_try_emit_from_nginput) を提供（リンク切れ防止＋将来拡張）
 */

#define DT_DRV_COMPAT zmk_behavior_mejiro

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h> // ARG_UNUSED

#include <drivers/behavior.h>
#include <zmk/behavior.h>

/* --- Mejiro public headers (あなたの規約: include/mejiro/...) ------------- */
#include "mejiro/mejiro_core.h"
#include "mejiro/mejiro_key_ids.h"

/* --- For naginata hook signature ----------------------------------------- */
/* NGListArray は zmk-naginata module 側の型。hook で参照するので include する */
#include <zmk_naginata/nglistarray.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* -------------------------------------------------------------------------- */
/* Internal state                                                             */
/* -------------------------------------------------------------------------- */

struct behavior_mejiro_config {
    /* reserved */
};

static struct {
    struct mejiro_state latched; /* 確定用スナップショット（押した履歴） */
    struct mejiro_state down;    /* 現在押下中（離されたか判定用） */
} g;

static inline void set_active_on_press(void) {
    g.latched.active = true;
    g.down.active = true;
}

static inline bool down_any(void) {
    return (g.down.left_mask | g.down.right_mask | g.down.mod_mask) != 0U;
}

static void record_key(enum mejiro_key_id id, bool pressed) {
    if (pressed) {
        set_active_on_press();

        /* latch: OR-in（履歴として保持） */
        mejiro_on_key_event(&g.latched, id, true);

        /* down: set（現在押下として保持） */
        mejiro_on_key_event(&g.down, id, true);
    } else {
        /* down: clear（現在押下から外す） */
        mejiro_on_key_event(&g.down, id, false);
    }
}

/* -------------------------------------------------------------------------- */
/* ZMK behavior callbacks                                                     */
/* -------------------------------------------------------------------------- */

static int behavior_mejiro_binding_pressed(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    enum mejiro_key_id id = (enum mejiro_key_id)binding->param1;
    record_key(id, true);

    /* ここではまだ送らない（離された瞬間に確定） */
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_mejiro_binding_released(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    enum mejiro_key_id id = (enum mejiro_key_id)binding->param1;
    record_key(id, false);

    /* 何も押されていない＝この瞬間に確定して送信 */
    if (!down_any() && g.latched.active) {
        /* ★ここが今まで欠けてた本体：確定ストロークを core に渡して送信する */
        (void)mejiro_try_emit(&g.latched);

        /* core が latched を reset していても安全のため両方 reset */
        mejiro_reset(&g.latched);
        mejiro_reset(&g.down);
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_mejiro_driver_api = {
    .binding_pressed = behavior_mejiro_binding_pressed,
    .binding_released = behavior_mejiro_binding_released,
};

/* -------------------------------------------------------------------------- */
/* Device instance                                                            */
/* -------------------------------------------------------------------------- */

#define BEHAVIOR_MEJIRO_INST(n)                                                \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL,                         \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
                            &behavior_mejiro_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BEHAVIOR_MEJIRO_INST)

/* -------------------------------------------------------------------------- */
/* Naginata hook (called from behavior_naginata.c)                             */
/* -------------------------------------------------------------------------- */

/**
 * Naginata 側で溜めた NGListArray を見て、
 * Mejiro が処理するなら Mejiro 側で送信して true（naginata 出力を抑止）
 * まだ処理しないなら false（従来どおり naginata に出力させる）
 *
 * ※今は「リンク切れ防止」と「呼ばれてる事実をログで掴む」最低限。
 * 　次段で nginput → mejiro_state 化 → mejiro_try_emit() に繋げる。
 */
bool mejiro_try_emit_from_nginput(const NGListArray *nginput, int64_t timestamp) {
    ARG_UNUSED(timestamp);

    if (!nginput) {
        return false;
    }

    LOG_DBG("MEJIRO hook: nginput size=%d", (int)nginput->size);

    /* TODO: nginput を解析して Mejiro の送信に変換する */
    return false;
}
