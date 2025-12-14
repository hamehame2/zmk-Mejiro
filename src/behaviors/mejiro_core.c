/*
 * mejiro_core.c
 *
 * Minimal Mejiro hook implementation:
 * - Receives NGListArray built by behavior_naginata.c
 * - If the chord is "Mejiro-only" (for now: Q/W/E markers), emit debug output
 *   via mej_output_utf8() and return true.
 * - Otherwise return false so behavior_naginata falls back to ng_type().
 *
 * NOTE: This is intentionally minimal to get builds passing first.
 */

#include <zephyr/kernel.h>

#include <zmk_naginata/nglist.h>
#include <zmk_naginata/nglistarray.h>
#include <zmk_naginata/naginata_func.h> /* Q/W/E symbols (and others if needed) */

#include <mejiro_core.h>

static bool has_key(const NGList *keys, uint32_t kc) {
    if (!keys) return false;
    for (int i = 0; i < keys->size; i++) {
        if (keys->elements[i] == kc) return true;
    }
    return false;
}

bool mejiro_try_emit_from_nginput(const NGListArray *nginput) {
    if (!nginput || nginput->size <= 0) return false;

    /* Current pipeline builds one "resolved chord" at elements[0]. */
    const NGList *chord = &nginput->elements[0];

    /* TODO: replace this section with real Mejiro "pure粒" detection (cvb / n m ,).
     * For now we keep a tiny smoke-test mapping so you can see the hook runs.
     */
    if (has_key(chord, Q)) { mej_output_utf8("mq"); return true; }
    if (has_key(chord, W)) { mej_output_utf8("mw"); return true; }
    if (has_key(chord, E)) { mej_output_utf8("me"); return true; }

    /* Not handled by Mejiro -> fall back to Naginata */
    return false;
}
