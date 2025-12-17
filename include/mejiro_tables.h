#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Tagged struct is REQUIRED because callers use `struct mj_kv` (not a typedef). */
struct mj_kv {
    const char *k;
    const char *v;
};

/* Minimal table surfaces used by mejiro_core. */
extern const struct mj_kv mj_commands[];
extern const size_t mj_commands_len;

extern const struct mj_kv mj_users[];
extern const size_t mj_users_len;

extern const struct mj_kv mj_abstract[];
extern const size_t mj_abstract_len;

extern const struct mj_kv mj_verbs[];
extern const size_t mj_verbs_len;

#ifdef __cplusplus
}
#endif
