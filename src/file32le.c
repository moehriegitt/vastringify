/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <stdio.h>
#include <assert.h>
#include "va_print/file.h"
#include "va_print/impl.h"

/* ********************************************************************** */
/* extern functions */

extern void va_file32_put_le(va_stream_t *s, char32_t c)
{
    va_stream_file_t *t = (va_stream_file_t*)s;
    if (fputc((char)(c & 0xff), t->file) < 0) {
        goto error;
    }
    if (fputc((char)((c >> 8) & 0xff), t->file) < 0) {
        goto error;
    }
    if (fputc((char)((c >> 16) & 0xff), t->file) < 0) {
        goto error;
    }
    if (fputc((char)((c >> 24) & 0xff), t->file) < 0) {
        goto error;
    }
    return;

error:
    va_stream_set_error(&t->s, VA_E_TRUNC);
}
