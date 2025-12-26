#pragma once
#include <stdbool.h>
#include <stdint.h>

/* behavior_mejiro から呼ぶ */
void mejiro_on_press(uint8_t key_id);
void mejiro_on_release(uint8_t key_id);

/* (naginata hook 等から “今のchord_maskをテーブル引きしたい” ため) */
bool mejiro_commit_now(void);
