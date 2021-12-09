/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/file.h"
#include "va_print/file_utf8.h"
#include "va_print/utf8.h"

/* ********************************************************************** */
/* static functions */

static void va_file_put_utf8(va_stream_t *s, unsigned c)
{
    va_put_utf8(s, c, va_file_put);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_file_vtab_utf8 = {
    .put = va_file_put_utf8
};
