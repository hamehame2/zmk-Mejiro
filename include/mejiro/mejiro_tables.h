#pragma once
/* Ensure the tagged struct is visible even if the included header is missing/old. */
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
struct mj_kv { const char *k; const char *v; };
#ifdef __cplusplus
}
#endif

#include "../mejiro_tables.h"
