#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* core state machine */
void mejiro_core_on_press(uint8_t key_id);
void mejiro_core_on_release(uint8_t key_id);
void mejiro_core_reset(void);

/* send_roman: core.c から呼ぶので宣言だけ置く（ヘッダ依存を増やさない） */
bool mejiro_send_roman_exec(const char *cmd);

#ifdef __cplusplus
}
#endif
