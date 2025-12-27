#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/logging/log.h>

#include <zmk_naginata/naginata_func.h> /* tapKey() / pressKey() / releaseKey() */

#include "mejiro/mejiro_send_roman.h"

LOG_MODULE_REGISTER(mejiro_send_roman, CONFIG_ZMK_LOG_LEVEL);

static bool tap_ascii(char c, int64_t timestamp) {
    /* zmk_naginata/naginata_func.h 側の keycode 定義（A,B,...,N1... 等）を使う */
    switch (c) {
    /* lower */
    case 'a': tapKey(A, timestamp); return true;
    case 'b': tapKey(B, timestamp); return true;
    case 'c': tapKey(C, timestamp); return true;
    case 'd': tapKey(D, timestamp); return true;
    case 'e': tapKey(E, timestamp); return true;
    case 'f': tapKey(F, timestamp); return true;
    case 'g': tapKey(G, timestamp); return true;
    case 'h': tapKey(H, timestamp); return true;
    case 'i': tapKey(I, timestamp); return true;
    case 'j': tapKey(J, timestamp); return true;
    case 'k': tapKey(K, timestamp); return true;
    case 'l': tapKey(L, timestamp); return true;
    case 'm': tapKey(M, timestamp); return true;
    case 'n': tapKey(N, timestamp); return true;
    case 'o': tapKey(O, timestamp); return true;
    case 'p': tapKey(P, timestamp); return true;
    case 'q': tapKey(Q, timestamp); return true;
    case 'r': tapKey(R, timestamp); return true;
    case 's': tapKey(S, timestamp); return true;
    case 't': tapKey(T, timestamp); return true;
    case 'u': tapKey(U, timestamp); return true;
    case 'v': tapKey(V, timestamp); return true;
    case 'w': tapKey(W, timestamp); return true;
    case 'x': tapKey(X, timestamp); return true;
    case 'y': tapKey(Y, timestamp); return true;
    case 'z': tapKey(Z, timestamp); return true;

    /* upper: Shift + letter */
    case 'A': tapKey(LS(A), timestamp); return true;
    case 'B': tapKey(LS(B), timestamp); return true;
    case 'C': tapKey(LS(C), timestamp); return true;
    case 'D': tapKey(LS(D), timestamp); return true;
    case 'E': tapKey(LS(E), timestamp); return true;
    case 'F': tapKey(LS(F), timestamp); return true;
    case 'G': tapKey(LS(G), timestamp); return true;
    case 'H': tapKey(LS(H), timestamp); return true;
    case 'I': tapKey(LS(I), timestamp); return true;
    case 'J': tapKey(LS(J), timestamp); return true;
    case 'K': tapKey(LS(K), timestamp); return true;
    case 'L': tapKey(LS(L), timestamp); return true;
    case 'M': tapKey(LS(M), timestamp); return true;
    case 'N': tapKey(LS(N), timestamp); return true;
    case 'O': tapKey(LS(O), timestamp); return true;
    case 'P': tapKey(LS(P), timestamp); return true;
    case 'Q': tapKey(LS(Q), timestamp); return true;
    case 'R': tapKey(LS(R), timestamp); return true;
    case 'S': tapKey(LS(S), timestamp); return true;
    case 'T': tapKey(LS(T), timestamp); return true;
    case 'U': tapKey(LS(U), timestamp); return true;
    case 'V': tapKey(LS(V), timestamp); return true;
    case 'W': tapKey(LS(W), timestamp); return true;
    case 'X': tapKey(LS(X), timestamp); return true;
    case 'Y': tapKey(LS(Y), timestamp); return true;
    case 'Z': tapKey(LS(Z), timestamp); return true;

    /* digits */
    case '0': tapKey(N0, timestamp); return true;
    case '1': tapKey(N1, timestamp); return true;
    case '2': tapKey(N2, timestamp); return true;
    case '3': tapKey(N3, timestamp); return true;
    case '4': tapKey(N4, timestamp); return true;
    case '5': tapKey(N5, timestamp); return true;
    case '6': tapKey(N6, timestamp); return true;
    case '7': tapKey(N7, timestamp); return true;
    case '8': tapKey(N8, timestamp); return true;
    case '9': tapKey(N9, timestamp); return true;

    /* minimal punctuation */
    case ' ': tapKey(SPC, timestamp); return true;
    case '-': tapKey(MINUS, timestamp); return true;
    case '_': tapKey(LS(MINUS), timestamp); return true;
    case '/': tapKey(SLASH, timestamp); return true;
    case '.': tapKey(DOT, timestamp); return true;
    case ',': tapKey(COMMA, timestamp); return true;

    default:
        return false;
    }
}

bool mejiro_send_text(const char *s, int64_t timestamp) {
    if (!s) {
        return false;
    }

    bool any = false;
    for (const char *p = s; *p; p++) {
        if (!tap_ascii(*p, timestamp)) {
            LOG_WRN("mejiro_send_roman: unsupported char=0x%02x", (unsigned char)*p);
            /* 未対応文字は無視（必要なら false で止めてもいい） */
        } else {
            any = true;
        }
    }
    return any;
}
