#pragma once

#include <stddef.h> /* size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* ここはあなたの現行宣言に合わせてください。
 * 少なくとも size_t を使う宣言があるなら <stddef.h> は必須。
 */

bool mejiro_tables_lookup(const char *stroke, const char **out_roman);

/* 例: stroke 正規化のAPIがあるなら size_t を使うのでこれもOK */
void mejiro_tables_normalize(const char *in, char *out, size_t out_len);

#ifdef __cplusplus
}
#endif
