/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <assert.h>
#include "va_print/alloc.h"
#include "va_print/impl.h"

/* ********************************************************************** */
/* extern functions */

extern void va_vec32_init(
    va_stream_t *s)
{
    va_stream_vec32_t *t = (va_stream_vec32_t*)s;
    if ((t->data != NULL) || (t->size == 0)) {
        return;
    }

    t->data = t->alloc(NULL, t->size, sizeof(*t->data));
    if (t->data == NULL) {
        va_stream_set_error(s, VA_E_TRUNC);
        t->pos = 0;
        t->size = 0;
        return;
    }

    t->pos = 0;
    t->data[0] = 0;
}

extern void va_vec32_put(
    va_stream_t *s,
    char32_t c)
{
    va_stream_vec32_t *t = (va_stream_vec32_t*)s;
    if (t->data == NULL) {
        return;
    }

    if ((t->pos + 1) >= t->size) {
        t->size *= 2;
        char32_t *new_data = t->alloc(t->data, t->size, sizeof(*t->data));
        if (new_data == NULL) {
            (void)t->alloc(t->data, 0, sizeof(*t->data));
            va_stream_set_error(s, VA_E_TRUNC);
            t->pos = 0;
            t->size = 0;
            t->data = NULL;
            return;
        }
        t->data = new_data;
    }

    t->data[t->pos] = c;
    t->data[t->pos+1] = 0;
    t->pos++;
}
