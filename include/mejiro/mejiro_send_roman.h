#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Send a roman (ASCII) string.
 * NOTE: This is currently a "build-safe" stub; it may only log.
 */
void mejiro_send_roman_exec(const char *s);

/**
 * Optional command interface (e.g. ENTER/BS etc).
 * Return true if handled.
 */
bool mejiro_send_roman_exec_cmd(const char *cmd);

#ifdef __cplusplus
}
#endif
