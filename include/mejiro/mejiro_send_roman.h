/*
 * SPDX-License-Identifier: MIT
 *
 * Send roman (ASCII) string through ZMK HID.
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns true if sent. */
bool mejiro_send_roman(const char *text);

#ifdef __cplusplus
}
#endif
