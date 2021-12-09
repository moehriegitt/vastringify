/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/char.h"
#include "va_print/char_utf32.h"
#include "va_print/utf32.h"

/* ********************************************************************** */
/* static functions */

static void va_char32_p_put_utf32(va_stream_t *s, unsigned c)
{
    va_put_utf32(s, c, va_char32_p_put);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_char32_p_vtab_utf32 = {
    .init = va_char32_p_init,
    .put = va_char32_p_put_utf32,
};
