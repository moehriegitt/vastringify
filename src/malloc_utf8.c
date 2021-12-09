/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/malloc.h"
#include "va_print/malloc_utf8.h"
#include "va_print/utf8.h"

/* ********************************************************************** */
/* static functions */

static void va_vec_put_utf8(va_stream_t *s, unsigned c)
{
    va_put_utf8(s, c, va_vec_put);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_vec_vtab_utf8 = {
    .init = va_vec_init,
    .put = va_vec_put_utf8,
};
