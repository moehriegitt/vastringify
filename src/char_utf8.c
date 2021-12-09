/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/char.h"
#include "va_print/char_utf8.h"
#include "va_print/utf8.h"

/* ********************************************************************** */
/* static functions */

static void va_char_p_put_utf8(va_stream_t *s, unsigned c)
{
    va_put_utf8(s, c, va_char_p_put);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_char_p_vtab_utf8 = {
    .init = va_char_p_init,
    .put = va_char_p_put_utf8,
};
