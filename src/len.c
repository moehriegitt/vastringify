/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <assert.h>
#include "va_print/len.h"

/* ********************************************************************** */
/* static functions */

static void va_len_put(va_stream_t *s, unsigned c)
{
    va_stream_len_t *t = (va_stream_len_t*)s;
    (void)c;
    t->pos++;
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_len_vtab = {
    .put = va_len_put,
};
