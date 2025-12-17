#pragma once

#include <stdbool.h>
#include <zmk_naginata/nglistarray.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Called by the Naginata hook when a chord is finalized. */
bool mejiro_try_emit_from_nginput(const NGListArray *nginput);

#ifdef __cplusplus
}
#endif
