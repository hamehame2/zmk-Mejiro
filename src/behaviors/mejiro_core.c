/*
 * mejiro_core_min_fixed.c
 *
 * Minimal Mejiro core:
 *   - Recognizes only one stroke: V+M+B+N (any order) => outputs "kyou"
 *   - Returns false otherwise
 *
 * This is meant to prove that:
 *   (A) key collection / stroke-finalization works (in behavior_naginata.c)
 *   (B) the output path works (mej_output_utf8 -> keycode events -> host output)
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zmk_naginata/nglist.h>
#include <dt-bindings/zmk/keys.h>

/* provided by mejiro_send_roman_fixed.c */
extern void mej_output_utf8(const char *s);

static bool has_key(const NGList *keys, uint32_t kc) {
    if (!keys) return false;
    for (int i = 0; i < keys->size; i++) {
        if (keys->keys[i] == kc) return true;
    }
    return false;
}

static bool is_exact_vmbn(const NGList *keys) {
    if (!keys || keys->size != 4) return false;
    return has_key(keys, V) && has_key(keys, M) && has_key(keys, B) && has_key(keys, N);
}

/* Called from behavior_naginata.c (or any wrapper) */
bool mej_type_once(const NGList *keys) {
    if (is_exact_vmbn(keys)) {
        mej_output_utf8("kyou");
        return true;
    }
    return false;
}
