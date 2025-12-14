/*
 * mejiro_core_min_kyou.c
 *
 * Minimal Mejiro core for validation:
 *   - Recognizes only one stroke: V+M+B+N (any order) => "kyou"
 *   - Returns false for everything else.
 *
 * Output is sent via mej_output_utf8(), which you should override with
 * mejiro_send_roman.c (press/release per character).
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <zmk_naginata/nglist.h>

/* keycode symbols like V, M, B, N */
#include <dt-bindings/zmk/keys.h>

/* Provided by sender layer (override weak) */
void mej_output_utf8(const char *s);

/* helper: does NGList contain keycode kc ? */
static bool has_key(const NGList *keys, uint32_t kc) {
    for (int i = 0; i < keys->size; i++) {
        if (keys->elements[i] == kc) return true;
    }
    return false;
}

/* helper: ensure no extra keys beyond the four (size==4) */
static bool is_exact_vmbn(const NGList *keys) {
    if (!keys || keys->size != 4) return false;
    return has_key(keys, V) && has_key(keys, M) && has_key(keys, B) && has_key(keys, N);
}

/* Called from behavior_naginata.c */
bool mej_type_once(const NGList *keys) {
    if (is_exact_vmbn(keys)) {
        mej_output_utf8("kyou");
        return true;
    }
    return false;
}
