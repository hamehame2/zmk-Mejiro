#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/keycode.h>
#include <zmk/hid.h>
#include "behavior_mejiro_min.h"

LOG_MODULE_REGISTER(mejiro_min, LOG_LEVEL_INF);

/* 対象キー: C V B N M COMMA */
enum {
    K_C = 0,
    K_V,
    K_B,
    K_N,
    K_M,
    K_COMMA,
    K_MAX
};

static uint8_t held[K_MAX];
static uint8_t held_count;

/* kc -> index。対象外は -1 */
static int kc_to_idx(uint32_t kc) {
    switch (kc) {
    case C:     return K_C;
    case V:     return K_V;
    case B:     return K_B;
    case N:     return K_N;
    case M:     return K_M;
    case COMMA: return K_COMMA;
    default:    return -1;
    }
}

static void clear_all(void) {
    for (int i = 0; i < K_MAX; i++) held[i] = 0;
    held_count = 0;
}

/* 仮の最小辞書：2エントリだけ */
static void try_emit(void) {
    /* 例1: C+V -> 'x' */
    if (held[K_C] && held[K_V] && held_count == 2) {
        zmk_hid_keyboard_press('x');
        zmk_hid_keyboard_release('x');
        return;
    }
    /* 例2: N+M -> 'y' */
    if (held[K_N] && held[K_M] && held_count == 2) {
        zmk_hid_keyboard_press('y');
        zmk_hid_keyboard_release('y');
        return;
    }
    /* 未定義は no-op */
}

void mejiro_min_press(uint32_t kc) {
    int idx = kc_to_idx(kc);
    if (idx < 0) return;          /* 対象外は完全無視 */
    if (!held[idx]) {
        held[idx] = 1;
        held_count++;
    }
}

void mejiro_min_release(uint32_t kc) {
    int idx = kc_to_idx(kc);
    if (idx < 0) return;

    if (held[idx]) {
        held[idx] = 0;
        if (held_count > 0) held_count--;
    }

    /* 全部離れた瞬間に1回だけ判定 */
    if (held_count == 0) {
        try_emit();
        clear_all();
    }
}
