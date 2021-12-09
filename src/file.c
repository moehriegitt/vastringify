/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <stdio.h>
#include <assert.h>
#include "va_print/file.h"
#include "va_print/impl.h"

/* ********************************************************************** */
/* extern functions */

extern void va_file_put(va_stream_t *s, char c)
{
    va_stream_file_t *t = (va_stream_file_t*)s;
    if (fputc(c, t->file) < 0) {
        va_stream_set_error(&t->s, VA_E_TRUNC);
    }
}
