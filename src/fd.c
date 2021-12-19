/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <unistd.h>
#include <assert.h>
#include "va_print/fd.h"
#include "va_print/impl.h"

/* ********************************************************************** */
/* extern functions */

extern void va_fd_put(va_stream_t *s, char c)
{
    va_stream_fd_t *t = (va_stream_fd_t*)s;
    if (write((int)t->fd, &c, 1) != 1) {
        va_stream_set_error(&t->s, VA_E_TRUNC);
    }
}
