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
 * - pressed_* は現在押されているキー(bit)
 * - latched_* は「ストローク中に関与したキー(bit)」を蓄積
 * - 最後のキーが離された瞬間(pressed_left/right == 0)に stroke を確定して辞書 lookup
 *
 * stroke 表現はあなたの方針に合わせる:
 * - 左だけ: "tk#" / "#" / "n" (右が無ければ '-' を入れない)
 * - 右だけ: "-U" / "-n" (右だけは '-' で始める)
 * - 両手: "STK-TR..." のように '-' を挟む
 * - H は '#'、X は '*'（左右キーとして分けない）
 */

/* ---- key_id -> bit mapping ------------------------------------------------
 * ここはあなたの実装に合わせて調整する場所。
 * いまは「key_id を 0..31 にして、そのまま 1u<<id を左右へ分配」という最小方針にしている。
 */
static inline uint32_t bit_of_key(uint16_t key_id) {
    if (key_id >= 32) {
        return 0;
    }
    return (1u << key_id);
}

/* ---- stroke string builder -----------------------------------------------
 * NOTE: ここは「テーブル lookup 用のキー文字列」なので、あなたの原典ルールに合わせて作る。
 * まずは最小で:
 * - left bits -> "stknya..." 等へ展開（順序固定）
 * - right bits -> "tr..." 等へ展開（順序固定）
 *
 * 実際の bit->文字 は、あなたの mejiro_key_ids / テーブルに合わせて埋める。
 */
static bool build_stroke_string(const struct mejiro_state *s, char *out, size_t out_len) {
    if (!s || !out || out_len == 0) {
        return false;
    }

    out[0] = '\0';

    const uint32_t L = s->latched_left;
    const uint32_t R = s->latched_right;

    /* TODO: ここをあなたの “左 S T K N Y I A U n t k (#/*)” へ確定させる */
    /* いまはデバッグ用に「L=xxxxxxxx R=xxxxxxxx」を返して lookup しない */
    /* ただしあなたが求めている確認ログ( stroke='...' )が出るようにするため、必ず何かを入れる */
    if (L == 0 && R == 0) {
        snprintf(out, out_len, "");
        return true;
    }

    /* 仮: 左だけ/右だけ/両手の '-' ルールだけ先に反映した “枠” を作る */
    if (L != 0 && R == 0) {
        /* left only: no leading '-' */
        snprintf(out, out_len, "L%08x", (unsigned int)L);
        return true;
    } else if (L == 0 && R != 0) {
        /* right only: leading '-' */
        snprintf(out, out_len, "-R%08x", (unsigned int)R);
        return true;
    } else {
        /* both: '-' between */
        snprintf(out, out_len, "L%08x-R%08x", (unsigned int)L, (unsigned int)R);
        return true;
    }
}

/* ---- public API ---------------------------------------------------------- */

void mejiro_reset(struct mejiro_state *s) {
    if (!s) {
        return;
    }
    s->active = false;
    s->pressed_left = 0;
    s->pressed_right = 0;
    s->latched_left = 0;
    s->latched_right = 0;
}

void mejiro_set_active(struct mejiro_state *s, bool on) {
    if (!s) {
        return;
    }
    s->active = on;
    if (!on) {
        s->pressed_left = 0;
        s->pressed_right = 0;
        s->latched_left = 0;
        s->latched_right = 0;
    }
}

bool mejiro_on_key_press(struct mejiro_state *s, uint16_t key_id, bool is_left, int64_t timestamp) {
    (void)timestamp;
    if (!s) {
        return false;
    }

    uint32_t bit = bit_of_key(key_id);
    if (bit == 0) {
        return false;
    }

    s->active = true;

    if (is_left) {
        s->pressed_left |= bit;
        s->latched_left |= bit;
    } else {
        s->pressed_right |= bit;
        s->latched_right |= bit;
    }

    return true;
}

bool mejiro_on_key_release(struct mejiro_state *s, uint16_t key_id, bool is_left, int64_t timestamp) {
    (void)timestamp;
    if (!s) {
        return false;
    }

    uint32_t bit = bit_of_key(key_id);
    if (bit == 0) {
        return false;
    }

    if (is_left) {
        s->pressed_left &= ~bit;
    } else {
        s->pressed_right &= ~bit;
    }

    return true;
}

bool mejiro_try_emit(struct mejiro_state *s, int64_t timestamp) {
    if (!s || !s->active) {
        return false;
    }

    /* まだ何か押されているなら確定しない */
    if (s->pressed_left != 0 || s->pressed_right != 0) {
        return false;
    }

    char stroke[128];
    if (!build_stroke_string(s, stroke, sizeof(stroke))) {
        /* 確定できないならリセットだけ */
        s->latched_left = 0;
        s->latched_right = 0;
        s->active = false;
        return false;
    }

    LOG_DBG("mejiro: commit stroke='%s' (L=0x%08x R=0x%08x)",
            stroke, (unsigned int)s->latched_left, (unsigned int)s->latched_right);

    /* 空は何もしない（ただリセット） */
    if (stroke[0] == '\0') {
        s->latched_left = 0;
        s->latched_right = 0;
        s->active = false;
        return true;
    }

    char out[128];
    if (!mejiro_tables_lookup(stroke, out, sizeof(out))) {
        LOG_DBG("mejiro: no command for stroke='%s'", stroke);
        s->latched_left = 0;
        s->latched_right = 0;
        s->active = false;
        return true;
    }

    LOG_DBG("mejiro: emit stroke='%s' -> '%s'", stroke, out);

    bool ok = mejiro_send_text(out, timestamp);

    /* 確定したので必ずリセット */
    s->latched_left = 0;
    s->latched_right = 0;
    s->active = false;

    return ok;
}
