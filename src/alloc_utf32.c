/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/alloc.h"
#include "va_print/alloc_utf32.h"
#include "va_print/utf32.h"

/* ********************************************************************** */
/* static functions */

static void va_vec32_put_utf32(va_stream_t *s, unsigned c)
{
    va_put_utf32(s, c, va_vec32_put);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_vec32_vtab_utf32 = {
    .init = va_vec32_init,
    .put = va_vec32_put_utf32,
};
