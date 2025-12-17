#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal Mejiro tables for ZMK module.
 *
 * This is intentionally tiny and dependency-free:
 * - Provide a stroke->string mapping table for "commands".
 * - Other tables (users/abstract/verbs) are provided as empty placeholders.
 *
 * Your mejiro_core.c can either:
 *   - use these arrays directly, or
 *   - call mejiro_lookup_* helpers below.
 */

struct mejiro_kv {
    const char *stroke;
    const char *out;
};

extern const struct mejiro_kv mj_commands[];
extern const size_t mj_commands_len;

extern const struct mejiro_kv mj_users[];
extern const size_t mj_users_len;

extern const struct mejiro_kv mj_abstract[];
extern const size_t mj_abstract_len;

extern const struct mejiro_kv mj_verbs[];
extern const size_t mj_verbs_len;

static inline const char *mejiro_lookup(const struct mejiro_kv *tab, size_t n, const char *stroke) {
    if (!tab || !stroke) return NULL;
    for (size_t i = 0; i < n; i++) {
        const char *k = tab[i].stroke;
        if (!k) continue;
        /* strcmp without pulling libc: do simple loop */
        const char *a = k;
        const char *b = stroke;
        while (*a && *b && (*a == *b)) { a++; b++; }
        if (*a == '\0' && *b == '\0') return tab[i].out;
    }
    return NULL;
}

static inline const char *mejiro_lookup_command(const char *stroke) {
    return mejiro_lookup(mj_commands, mj_commands_len, stroke);
}

#ifdef __cplusplus
}
#endif
