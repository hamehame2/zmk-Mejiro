#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Send a roman string (ASCII) to host.
 * This function is "fire and forget" and returns void.
 */
void mejiro_send_roman_exec(const char *s);

/**
 * Execute a simple command string if you are using a command-like layer.
 * Returns true if the command was recognized and handled.
 */
bool mejiro_send_roman_exec_cmd(const char *cmd);

#ifdef __cplusplus
}
#endif
