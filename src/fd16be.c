/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <unistd.h>
#include <assert.h>
#include "va_print/fd.h"
#include "va_print/impl.h"

/* ********************************************************************** */
/* extern functions */

extern void va_fd16_put_be(va_stream_t *s, char16_t c)
{
    va_stream_fd_t *t = (va_stream_fd_t*)s;
    unsigned char x[2];
    x[0] = (unsigned char)(c >> 8);
    x[1] = (unsigned char)(c & 0xff);
    if (write((int)t->fd, &x, 2) != 2) {
        va_stream_set_error(&t->s, VA_E_TRUNC);
    }
}
