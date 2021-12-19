/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <unistd.h>
#include <assert.h>
#include "va_print/fd.h"
#include "va_print/impl.h"

/* ********************************************************************** */
/* extern functions */

extern void va_fd32_put_be(va_stream_t *s, char32_t c)
{
    va_stream_fd_t *t = (va_stream_fd_t*)s;
    unsigned char x[4];
    x[0] = (unsigned char)((c >> 24) & 0xff);
    x[1] = (unsigned char)((c >> 16) & 0xff);
    x[2] = (unsigned char)((c >> 8) & 0xff);
    x[3] = (unsigned char)(c & 0xff);
    if (write((int)t->fd, &x, 4) != 4) {
        va_stream_set_error(&t->s, VA_E_TRUNC);
    }
}
