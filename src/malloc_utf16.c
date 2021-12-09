/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/malloc.h"
#include "va_print/malloc_utf16.h"
#include "va_print/utf16.h"

/* ********************************************************************** */
/* static functions */

static void va_vec16_put_utf16(va_stream_t *s, unsigned c)
{
    va_put_utf16(s, c, va_vec16_put);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_vec16_vtab_utf16 = {
    .init = va_vec16_init,
    .put = va_vec16_put_utf16,
};
