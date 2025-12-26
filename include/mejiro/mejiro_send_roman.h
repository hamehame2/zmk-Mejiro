#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Send a roman string (ASCII) to host.
 * fire-and-forget.
 */
void mejiro_send_roman_exec(const char *s);

/**
 * Execute a simple command-like string.
 * Returns true if handled.
 */
bool mejiro_send_roman_exec_cmd(const char *cmd);

#ifdef __cplusplus
}
#endif
