#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Convert a built stroke string to output text.
 * Return true if matched.
 *
 * out must be NUL-terminated when true.
 */
bool mejiro_tables_lookup(const char *stroke, char *out, size_t out_len);

#ifdef __cplusplus
}
#endif
