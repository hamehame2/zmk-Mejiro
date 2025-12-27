/*
 * SPDX-License-Identifier: MIT
 */
#include "mejiro/mejiro_tables.h"

#include <string.h>

/* 最小：動作確認用。必要なら後で増やす */
struct entry {
    const char *stroke;
    const char *out;
};

static const struct entry k_table[] = {
    /* 例: 左だけ */
    {"t", "t"},
    {"k", "k"},
    {"n", "n"},

    /* 例: 右だけ（右だけは '-' 始まりのルールに合わせる） */
    {"-t", "t"},
    {"-k", "k"},
    {"-n", "n"},

    /* 例: 両手（'-' 挟む） */
    {"t-k", "tk"},
    {"k-t", "kt"},

    /* 例: mod */
    {"t#", "t"}, /* '#' は装飾扱いにしても良い。ここは仮 */
};

bool mejiro_tables_lookup(const char *stroke, const char **out) {
    if (!stroke || !out) {
        return false;
    }

    for (size_t i = 0; i < (sizeof(k_table) / sizeof(k_table[0])); i++) {
        if (strcmp(k_table[i].stroke, stroke) == 0) {
            *out = k_table[i].out;
            return true;
        }
    }
    return false;
}
