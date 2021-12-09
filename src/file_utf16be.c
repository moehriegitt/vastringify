/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/file.h"
#include "va_print/file_utf16be.h"
#include "va_print/utf16.h"

/* ********************************************************************** */
/* static functions */

static void va_file16_put_utf16be(va_stream_t *s, unsigned c)
{
    va_put_utf16(s, c, va_file16_put_be);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_file16_vtab_utf16be = {
    .put = va_file16_put_utf16be
};
