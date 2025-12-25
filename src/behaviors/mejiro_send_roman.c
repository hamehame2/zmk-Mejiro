/* -------------------------------------------------------------------------
 * FILE: src/behaviors/mejiro_send_roman.c
 * ------------------------------------------------------------------------- */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <string.h>

#include <zmk/endpoints.h>
#include <zmk/hid.h>
#include <dt-bindings/zmk/keys.h>


LOG_MODULE_REGISTER(mejiro_send_roman, CONFIG_ZMK_LOG_LEVEL);

/* Minimal executor:
 * - supports "{#BackSpace}", "{#Delete}", "{#Left}", "{#Right}", "{#Up}", "{#Down}",
 *             "{#Home}", "{#End}", "{#Escape}", "{#F13}", "{#F14}"
 * - supports "=undo" (Ctrl+Z)
 * - everything else: return false (no-op)
 *
 * NOTE:
 * ここも「正攻法」で増やしていく前提。まずは tables の mj_commands にある最低限だけ。
 */

static void tap(zmk_key_t keycode) {
    zmk_hid_keyboard_press(keycode);
    zmk_hid_keyboard_release(keycode);
}

static void tap_with_mod(zmk_key_t mod_keycode, zmk_key_t keycode) {
    zmk_hid_keyboard_press(mod_keycode);
    zmk_hid_keyboard_press(keycode);
    zmk_hid_keyboard_release(keycode);
    zmk_hid_keyboard_release(mod_keycode);
}

static bool match_token(const char *s, const char *tok) {
    return s && tok && (strcmp(s, tok) == 0);
}

static bool exec_brace_hash(const char *token) {
    /* token is like "#BackSpace" */
    if (match_token(token, "#BackSpace")) {
        tap(ZMK_KEY_BACKSPACE);
        return true;
    }
    if (match_token(token, "#Delete")) {
        tap(ZMK_KEY_DELETE);
        return true;
    }
    if (match_token(token, "#Left")) {
        tap(ZMK_KEY_LEFT);
        return true;
    }
    if (match_token(token, "#Right")) {
        tap(ZMK_KEY_RIGHT);
        return true;
    }
    if (match_token(token, "#Up")) {
        tap(ZMK_KEY_UP);
        return true;
    }
    if (match_token(token, "#Down")) {
        tap(ZMK_KEY_DOWN);
        return true;
    }
    if (match_token(token, "#Home")) {
        tap(ZMK_KEY_HOME);
        return true;
    }
    if (match_token(token, "#End")) {
        tap(ZMK_KEY_END);
        return true;
    }
    if (match_token(token, "#Escape")) {
        tap(ZMK_KEY_ESCAPE);
        return true;
    }
    if (match_token(token, "#F13")) {
        tap(ZMK_KEY_F13);
        return true;
    }
    if (match_token(token, "#F14")) {
        tap(ZMK_KEY_F14);
        return true;
    }

    return false;
}

/* Parse only:
 *   "{#TOKEN}" exactly
 * For strings like "{^[]^}{#Left}{^}" we just execute the middle {#Left} and ignore the rest.
 */
static bool exec_first_hash_group(const char *cmd) {
    const char *p = cmd;
    while (p && *p) {
        if (*p == '{') {
            const char *q = strchr(p, '}');
            if (!q) break;

            /* extract inside braces */
            char inside[64];
            size_t len = (size_t)(q - (p + 1));
            if (len >= sizeof(inside)) len = sizeof(inside) - 1;
            memcpy(inside, p + 1, len);
            inside[len] = '\0';

            if (inside[0] == '#') {
                return exec_brace_hash(inside);
            }

            p = q + 1;
            continue;
        }
        p++;
    }
    return false;
}

bool mejiro_send_roman_exec(const char *cmd) {
    if (!cmd || !*cmd) {
        return false;
    }

    if (cmd[0] == '=') {
        if (strcmp(cmd, "=undo") == 0) {
            tap_with_mod(ZMK_KEY_LCTRL, ZMK_KEY_Z);
            return true;
        }
        /* "=repeat_last_translation" etc: not implemented yet */
        LOG_DBG("unsupported '=' command: %s", cmd);
        return false;
    }

    if (cmd[0] == '{') {
        if (exec_first_hash_group(cmd)) {
            return true;
        }
        LOG_DBG("unsupported brace command: %s", cmd);
        return false;
    }

    /* literal text typing: not implemented in this minimal step */
    LOG_DBG("unsupported literal command: %s", cmd);
    return false;
}
