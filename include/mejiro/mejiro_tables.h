/*
 * SPDX-License-Identifier: MIT
 *
 * Stroke string -> output mapping.
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Return true if found. out must be NUL-terminated on success. */
bool mejiro_tables_lookup(const char *stroke, const char **out);

#ifdef __cplusplus
}
#endif
