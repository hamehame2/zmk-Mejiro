#pragma once
#include <stdbool.h>

/* Send roman string through ZMK (implementation uses zmk_hid / behavior layer). */
bool mejiro_send_roman(const char *roman);
