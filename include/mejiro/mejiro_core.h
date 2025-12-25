/* -------------------------------------------------------------------------
 * FILE: include/mejiro/mejiro_core.h
 * ------------------------------------------------------------------------- */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* 7 keys exposed via DT binding cells (dt-bindings/zmk/mejiro.h):
 *   MJ_H, MJ_TL, MJ_NL, MJ_TR, MJ_NR, MJ_KL, MJ_KR
 *
 * We treat them as bit positions 0..6.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Called from behavior driver */
void mejiro_core_on_press(uint8_t key_id);
void mejiro_core_on_release(uint8_t key_id);

/* If you want to force flush (e.g. layer change) */
void mejiro_core_reset(void);

#ifdef __cplusplus
}
#endif
