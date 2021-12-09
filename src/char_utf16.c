/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/char.h"
#include "va_print/char_utf16.h"
#include "va_print/utf16.h"

/* ********************************************************************** */
/* static functions */

static void va_char16_p_put_utf16(va_stream_t *s, unsigned c)
{
    va_put_utf16(s, c, va_char16_p_put);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_char16_p_vtab_utf16 = {
    .init = va_char16_p_init,
    .put = va_char16_p_put_utf16,
};
