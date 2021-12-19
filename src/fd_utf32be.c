/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/fd.h"
#include "va_print/fd_utf32be.h"
#include "va_print/utf32.h"

/* ********************************************************************** */
/* static functions */

static void va_fd32_put_utf32be(va_stream_t *s, unsigned c)
{
    va_put_utf32(s, c, va_fd32_put_be);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_fd32_vtab_utf32be = {
    .put = va_fd32_put_utf32be
};
