#pragma once

#include <stdbool.h>
#include <zmk_naginata/nglistarray.h>

/* Called from behavior_naginata.c on key release when a full chord is ready.
 * Return true if Mejiro handled the chord and emitted output.
 */
bool mejiro_try_emit_from_nginput(const NGListArray *nginput);

/* UTF-8 output backend (roman/ASCII for now) implemented in mejiro_send_roman.c */
void mej_output_utf8(const char *s);
