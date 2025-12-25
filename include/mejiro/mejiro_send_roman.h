/* -------------------------------------------------------------------------
 * FILE: include/mejiro/mejiro_send_roman.h
 * ------------------------------------------------------------------------- */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Execute command string like:
 *   "{#BackSpace}", "{^[]^}{#Left}{^}", "=undo", etc.
 * Return true if executed, false if unsupported/no-op.
 */
bool mejiro_send_roman_exec(const char *cmd);

#ifdef __cplusplus
}
#endif
