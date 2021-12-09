/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <assert.h>
#include "va_print/char.h"
#include "va_print/impl.h"

/* ********************************************************************** */
/* extern functions */

extern void va_char32_p_init(va_stream_t *s)
{
    va_stream_char_p_t *t = (va_stream_char_p_t*)s;
    char32_t *data = t->data;
    if (t->pos >= t->size) {
        va_stream_set_error(&t->s, VA_E_TRUNC);
        return;
    }
    data[t->pos] = 0;
}

extern void va_char32_p_put(va_stream_t *s, char32_t c)
{
    va_stream_char_p_t *t = (va_stream_char_p_t*)s;
    char32_t *data = t->data;
    if (t->pos + 1 >= t->size) {
        va_stream_set_error(&t->s, VA_E_TRUNC);
        return;
    }
    data[t->pos] = c;
    data[++t->pos] = 0;
}
