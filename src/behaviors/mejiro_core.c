#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "mejiro/mejiro_core.h"
#include "mejiro/mejiro_tables.h"
#include "mejiro/mejiro_send_roman.h"

LOG_MODULE_REGISTER(mejiro_core, CONFIG_ZMK_LOG_LEVEL);

/*
 * 重要:
 * - pressed_mask: 現在 “押されている” キー集合
 * - latched_mask: ストローク確定までに “一度でも押された” キー集合（離しても保持）
 *
 * 最後のキーが離された瞬間 (pressed_mask == 0) に latched_mask を stroke 化して辞書 lookup。
 * その後 reset。
 */

static void build_stroke_string(uint32_t left_mask, uint32_t right_mask,
                               char *buf, size_t buflen) {
    /* 例: "L:.... R:...." みたいにしても良いが、今回は最小で疎通優先 */
    /* stroke 文字列は辞書側と一致させる必要があるので、
       まずは固定フォーマットで簡単にする */
    if (!buf || buflen == 0) return;

    /* 固定: "L%08x-R%08x" */
    (void)snprintk(buf, buflen, "L%08x-R%08x", (unsigned)left_mask, (unsigned)right_mask);
}

void mejiro_reset(struct mejiro_state *s) {
    if (!s) return;
    s->active = false;
    s->pressed_left = 0;
    s->pressed_right = 0;
    s->latched_left = 0;
    s->latched_right = 0;
}

void mejiro_set_active(struct mejiro_state *s, bool active) {
    if (!s) return;
    s->active = active;

    /* OFF にした瞬間はバッファ破棄 */
    if (!active) {
        s->pressed_left = 0;
        s->pressed_right = 0;
        s->latched_left = 0;
        s->latched_right = 0;
    }
}

static inline uint32_t bit_of_key(uint8_t key_id) {
    if (key_id >= 32) return 0;
    return (1u << key_id);
}

void mejiro_on_key_press(struct mejiro_state *s, bool is_left, uint8_t key_id) {
    if (!s || !s->active) return;

    uint32_t bit = bit_of_key(key_id);
    if (!bit) return;

    if (is_left) {
        s->pressed_left |= bit;
        s->latched_left |= bit;
    } else {
        s->pressed_right |= bit;
        s->latched_right |= bit;
    }
}

void mejiro_on_key_release(struct mejiro_state *s, bool is_left, uint8_t key_id) {
    if (!s || !s->active) return;

    uint32_t bit = bit_of_key(key_id);
    if (!bit) return;

    if (is_left) {
        s->pressed_left &= ~bit;
    } else {
        s->pressed_right &= ~bit;
    }
}

bool mejiro_try_emit(struct mejiro_state *s, int64_t timestamp) {
    if (!s || !s->active) {
        return false;
    }

    /* まだ押されているキーがあるなら確定しない */
    if ((s->pressed_left | s->pressed_right) != 0) {
        return false;
    }

    /* latched が空なら何もしない */
    if ((s->latched_left | s->latched_right) == 0) {
        return false;
    }

    char stroke[64];
    build_stroke_string(s->latched_left, s->latched_right, stroke, sizeof(stroke));

    char out[64];
    if (!mejiro_tables_lookup(stroke, out, sizeof(out))) {
        LOG_DBG("mejiro: no match stroke=%s", stroke);
        s->latched_left = 0;
        s->latched_right = 0;
        return false;
    }

    /* 空文字は “出力なし扱い” で消費して終わり（必要ならここを変更） */
    if (out[0] == '\0') {
        LOG_DBG("mejiro: matched but empty out stroke=%s", stroke);
        s->latched_left = 0;
        s->latched_right = 0;
        return true;
    }

    LOG_DBG("mejiro: emit stroke=%s -> %s", stroke, out);

    bool ok = mejiro_send_text(out, timestamp);

    /* 確定したので必ずリセット */
    s->latched_left = 0;
    s->latched_right = 0;

    return ok;
}
