#pragma once
/*
 * SPDX-License-Identifier: MIT
 *
 * Roman output (ASCII -> HID key taps)
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

bool mejiro_send_text(const char *s, int64_t timestamp);

#ifdef __cplusplus
}
#endif
