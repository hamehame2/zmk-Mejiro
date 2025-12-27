#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "mejiro/mejiro_tables.h"

/*
 * 今は “コンパイルが通る” スタブ。
 * 後でここを「stroke → かな/ローマ字」辞書に差し替える。
 *
 * Mejiro の Python 側ロジックは stroke を正規表現で分解して
 * kana/助詞/略語を合成してる（あなたが添付した mejiro_base.py / func.py）。
 * :contentReference[oaicite:1]{index=1} :contentReference[oaicite:2]{index=2}
 */
bool mejiro_tables_lookup(const char *stroke, char *out, size_t out_len) {
    if (!stroke || !out || out_len == 0) {
        return false;
    }

    /* Example: a hardcoded test mapping */
    if (strcmp(stroke, "STKNYIAUntk-STKNYIAUntk") == 0) {
        /* "出力取止" 相当（Python側も特別扱いしてる） :contentReference[oaicite:3]{index=3} */
        out[0] = '\0';
        return true;
    }

    /* no match */
    return false;
}
