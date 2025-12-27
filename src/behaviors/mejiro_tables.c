/*
 * SPDX-License-Identifier: MIT
 *
 * Minimal bring-up tables for Mejiro:
 * - Converts masks to a simple ASCII string so we can confirm output.
 *
 * Later you will replace this with real Mejiro mapping.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const char left_chars[16]  = { 'a','s','d','f','g','h','j','k','l','q','w','e','r','t','y','u' };
static const char right_chars[16] = { 'n','m',',','.','/','z','x','c','v','b','p','o','i','0','1','2' };

/*
 * Build an output string from masks (deterministic order).
 * out must be large enough. returns out length.
 */
size_t mejiro_tables_build_output(uint32_t left_mask, uint32_t right_mask, uint32_t mod_mask,
                                  char *out, size_t out_cap) {
    (void)mod_mask;

    if (!out || out_cap == 0) {
        return 0;
    }

    size_t w = 0;

    /* left */
    for (int i = 0; i < 16; i++) {
        if (left_mask & (1u << i)) {
            if (w + 1 >= out_cap) break;
            out[w++] = left_chars[i];
        }
    }

    /* right */
    for (int i = 0; i < 16; i++) {
        if (right_mask & (1u << i)) {
            if (w + 1 >= out_cap) break;
            out[w++] = right_chars[i];
        }
    }

    out[w] = '\0';
    return w;
}
