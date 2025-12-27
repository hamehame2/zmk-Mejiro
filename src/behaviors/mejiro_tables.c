#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "mejiro/mejiro_tables.h"

/*
 * まず疎通確認:
 *  - 何か stroke が来たら必ず "a" を返す
 * これで「&mj で無音」が “ルーティング/確定/送信” のどこで止まってるかを切り分けできる。
 *
 * 疎通が取れたら、ここを本物の辞書に差し替える。
 */
bool mejiro_tables_lookup(const char *stroke, char *out, size_t out_len) {
    if (!stroke || !out || out_len == 0) {
        return false;
    }

    /* 空 stroke は不一致 */
    if (stroke[0] == '\0') {
        return false;
    }

    /* 疎通用: 常に "a" */
    if (out_len < 2) {
        return false;
    }
    out[0] = 'a';
    out[1] = '\0';
    return true;
}
