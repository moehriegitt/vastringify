/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/fd.h"
#include "va_print/fd_utf16le.h"
#include "va_print/utf16.h"

/* ********************************************************************** */
/* static functions */

static void va_fd16_put_utf16le(va_stream_t *s, unsigned c)
{
    va_put_utf16(s, c, va_fd16_put_le);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_fd16_vtab_utf16le = {
    .put = va_fd16_put_utf16le
};
