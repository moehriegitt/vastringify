/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <assert.h>
#include <stdlib.h>
#include "va_print/alloc.h"

/* ********************************************************************** */
/* extern functions */

extern void *va_alloc(
    void *data,
    size_t nmemb,
    size_t size)
{
    assert(size != 0);
    if (nmemb == 0) {
        free(data);
        return NULL;
    }

#if 0
    size_t total = nmemb * size;
    if ((total / size) != nmemb) {
        return NULL;
    }
#else
    size_t total;
    if (__builtin_mul_overflow(nmemb, size, &total)) {
        return NULL;
    }
#endif
    return realloc(data, total);
}
