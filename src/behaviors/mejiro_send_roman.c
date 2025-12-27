#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/logging/log.h>

#include <zmk/hid.h>
#include <dt-bindings/zmk/keys.h>

#include "mejiro/mejiro_send_roman.h"

LOG_MODULE_REGISTER(mejiro_send_roman, CONFIG_ZMK_LOG_LEVEL);

/* ------------------------------------------------------------------------- */
/* Minimal self-contained tap helper (NO naginata dependency)                 */
/* ------------------------------------------------------------------------- */
static inline bool tap_key(uint32_t usage) {
    int r1 = zmk_hid_press(usage);
    int r2 = zmk_hid_release(usage);
    if (r1 < 0 || r2 < 0) {
        LOG_ERR("tap_key failed: usage=0x%08x r1=%d r2=%d", usage, r1, r2);
        return false;
    }
    return true;
}

static bool tap_ascii(char c, int64_t timestamp) {
    (void)timestamp;

    switch (c) {
    /* lower */
    case 'a': return tap_key(A);
    case 'b': return tap_key(B);
    case 'c': return tap_key(C);
    case 'd': return tap_key(D);
    case 'e': return tap_key(E);
    case 'f': return tap_key(F);
    case 'g': return tap_key(G);
    case 'h': return tap_key(H);
    case 'i': return tap_key(I);
    case 'j': return tap_key(J);
    case 'k': return tap_key(K);
    case 'l': return tap_key(L);
    case 'm': return tap_key(M);
    case 'n': return tap_key(N);
    case 'o': return tap_key(O);
    case 'p': return tap_key(P);
    case 'q': return tap_key(Q);
    case 'r': return tap_key(R);
    case 's': return tap_key(S);
    case 't': return tap_key(T);
    case 'u': return tap_key(U);
    case 'v': return tap_key(V);
    case 'w': return tap_key(W);
    case 'x': return tap_key(X);
    case 'y': return tap_key(Y);
    case 'z': return tap_key(Z);

    /* upper: use Shift + letter */
    case 'A': tap_key(LSHFT); return tap_key(A);
    case 'B': tap_key(LSHFT); return tap_key(B);
    case 'C': tap_key(LSHFT); return tap_key(C);
    case 'D': tap_key(LSHFT); return tap_key(D);
    case 'E': tap_key(LSHFT); return tap_key(E);
    case 'F': tap_key(LSHFT); return tap_key(F);
    case 'G': tap_key(LSHFT); return tap_key(G);
    case 'H': tap_key(LSHFT); return tap_key(H);
    case 'I': tap_key(LSHFT); return tap_key(I);
    case 'J': tap_key(LSHFT); return tap_key(J);
    case 'K': tap_key(LSHFT); return tap_key(K);
    case 'L': tap_key(LSHFT); return tap_key(L);
    case 'M': tap_key(LSHFT); return tap_key(M);
    case 'N': tap_key(LSHFT); return tap_key(N);
    case 'O': tap_key(LSHFT); return tap_key(O);
    case 'P': tap_key(LSHFT); return tap_key(P);
    case 'Q': tap_key(LSHFT); return tap_key(Q);
    case 'R': tap_key(LSHFT); return tap_key(R);
    case 'S': tap_key(LSHFT); return tap_key(S);
    case 'T': tap_key(LSHFT); return tap_key(T);
    case 'U': tap_key(LSHFT); return tap_key(U);
    case 'V': tap_key(LSHFT); return tap_key(V);
    case 'W': tap_key(LSHFT); return tap_key(W);
    case 'X': tap_key(LSHFT); return tap_key(X);
    case 'Y': tap_key(LSHFT); return tap_key(Y);
    case 'Z': tap_key(LSHFT); return tap_key(Z);

    /* digits */
    case '0': return tap_key(N0);
    case '1': return tap_key(N1);
    case '2': return tap_key(N2);
    case '3': return tap_key(N3);
    case '4': return tap_key(N4);
    case '5': return tap_key(N5);
    case '6': return tap_key(N6);
    case '7': return tap_key(N7);
    case '8': return tap_key(N8);
    case '9': return tap_key(N9);

    /* whitespace/control */
    case ' ': return tap_key(SPACE);
    case '\n': return tap_key(ENTER);
    case '\t': return tap_key(TAB);

    /* common punctuation (US) */
    case '-': return tap_key(MINUS);
    case '_': tap_key(LSHFT); return tap_key(MINUS);

    case '=': return tap_key(EQUAL);
    case '+': tap_key(LSHFT); return tap_key(EQUAL);

    case '[': return tap_key(LBKT);
    case '{': tap_key(LSHFT); return tap_key(LBKT);

    case ']': return tap_key(RBKT);
    case '}': tap_key(LSHFT); return tap_key(RBKT);

    case '\\': return tap_key(BSLH);
    case '|': tap_key(LSHFT); return tap_key(BSLH);

    case ';': return tap_key(SEMI);
    case ':': tap_key(LSHFT); return tap_key(SEMI);

    case '\'': return tap_key(SQT);
    case '"': tap_key(LSHFT); return tap_key(SQT);

    case ',': return tap_key(COMMA);
    case '<': tap_key(LSHFT); return tap_key(COMMA);

    case '.': return tap_key(DOT);
    case '>': tap_key(LSHFT); return tap_key(DOT);

    case '/': return tap_key(FSLH);
    case '?': tap_key(LSHFT); return tap_key(FSLH);

    case '`': return tap_key(GRAVE);
    case '~': tap_key(LSHFT); return tap_key(GRAVE);

    case '!': tap_key(LSHFT); return tap_key(N1);
    case '@': tap_key(LSHFT); return tap_key(N2);
    case '#': tap_key(LSHFT); return tap_key(N3);
    case '$': tap_key(LSHFT); return tap_key(N4);
    case '%': tap_key(LSHFT); return tap_key(N5);
    case '^': tap_key(LSHFT); return tap_key(N6);
    case '&': tap_key(LSHFT); return tap_key(N7);
    case '*': tap_key(LSHFT); return tap_key(N8);
    case '(': tap_key(LSHFT); return tap_key(N9);
    case ')': tap_key(LSHFT); return tap_key(N0);

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
        } else {
            any = true;
        }
    }
    return any;
}
