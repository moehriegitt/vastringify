/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include "va_print/fd.h"
#include "va_print/fd_utf8.h"
#include "va_print/utf8.h"

/* ********************************************************************** */
/* static functions */

static void va_fd_put_utf8(va_stream_t *s, unsigned c)
{
    va_put_utf8(s, c, va_fd_put);
}

/* ********************************************************************** */
/* extern objects */

va_stream_vtab_t const va_fd_vtab_utf8 = {
    .put = va_fd_put_utf8
};
