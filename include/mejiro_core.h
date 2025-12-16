#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <zmk_naginata/nglistarray.h>
bool mejiro_try_emit_from_nginput(const NGListArray *nginput, int64_t timestamp);
