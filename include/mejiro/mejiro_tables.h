#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "mejiro_key_ids.h"

/*
 * Convert stroke (left/right/mod masks) into a normalized stroke string,
 * then lookup output string.
 *
 * Returns true and sets out_roman if matched.
 */
bool mejiro_lookup_roman(uint32_t left_mask, uint32_t right_mask, uint32_t mod_mask,
                        const char **out_roman);

/* Build normalized stroke string mainly for debugging/logging. */
void mejiro_build_stroke_string(uint32_t left_mask, uint32_t right_mask, uint32_t mod_mask,
                               char *out, size_t out_len);
