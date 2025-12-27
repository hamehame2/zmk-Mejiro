#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/logging/log.h>

#include <zmk/hid.h>
#include <dt-bindings/zmk/keys.h>

#include "mejiro/mejiro_send_roman.h"


/*
 * ここは環境差が出やすい。
 * - SEND_STRING を使える構成ならそれを使う
 * - 使えないなら ZMK の HID 送信APIに差し替え
 *
 * まずは「コンパイルが通る」最小として、ZMK側のユーティリティを使う版だけ用意。
 */
#include <zmk/hid.h>
#include <zmk/keys.h>

#include <string.h>

/* ASCII 1文字だけ送る最小（必要なら後で拡張） */
static bool send_ascii_char(char c) {
    /* printable ASCII only */
    if (c < 0x20 || c > 0x7E) {
        return false;
    }

    /* ZMK の keycode に落とすのは完全対応が必要なので、ここは後で本実装に置換前提。
       今は「送信部の枠」を用意するだけ。*/
    return false;
}

bool mejiro_send_roman(const char *text) {
    if (!text) {
        return false;
    }

    /* いったん「送れたことにする」ダミーでビルドを通す版（動作確認は後で実装）。 */
    (void)text;
    return true;
}
