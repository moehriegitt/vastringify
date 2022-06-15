/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <stddef.h>
#include <assert.h>
#include <va_print/char.h>
#include <va_print/utf32.h>
#include <va_print/core.h>
#include <va_print/impl.h>

/* ********************************************************************** */
/* extern objects */

va_read_iter_vtab_t const va_char32_p_read_vtab_utf32 = {
    "char32_t*",
    va_char32_p_take_utf32,
    va_char32_p_end,
    'U',
    {0}
};

va_read_iter_vtab_t const va_arr32_p_read_vtab_utf32 = {
    "char32_t*",
    va_arr32_p_take_utf32,
    va_char32_p_end,
    'U',
    {0}
};

/* ********************************************************************** */
/* extern functions */

extern unsigned va_char32_p_take_utf32(
    va_read_iter_t *iter,
    void const *end)
{
    assert(iter->cur != NULL);
    char32_t const *s = iter->cur;
    unsigned c0 = (iter->cur == end) ? 0 : *s;
    if (c0 == 0) {
        /* end of string */
        return 0;
    }

    iter->cur = (s + 1);
    if (va_u_valid(c0)) {
        return c0;
    }

    if (c0 > VA_U_MAXMAX) {
        /* There is no way we can reproduce the same value, unfortunately,
         * because there's not enough space in the internal representation
         * of characters. */
        return VA_U_REPLACEMENT;
    }

    return c0 | VA_U_ENC_UTF32;
}

extern void va_put_utf32(
    va_stream_t *s,
    unsigned c,
    void (*put)(va_stream_t *, char32_t))
{
    /* decoder errors */
    if ((c & VA_U_ENC) != 0) {
        if ((c & VA_U_ENC) == VA_U_ENC_UTF32) {
            /* pass-through erroneous word */
            put(s, c & VA_U_DATA);
            return;
        }
        c = VA_U_REPLACEMENT;
    }

    /* encoding errors */
    if (!va_u_valid(c)) {
        va_stream_set_error(s, VA_E_ENCODE);
        c = VA_U_REPLACEMENT;
    }

    put(s, c);
}

extern va_stream_t *va_xprintf_char32_p_utf32(
    va_stream_t *s,
    char32_t const *x)
{
    va_read_iter_t iter = VA_READ_ITER(&va_char32_p_read_vtab_utf32, x);
    return va_xprintf_iter(s, &iter);
}

extern va_stream_t *va_xprintf_char32_const_pp_utf32(
    va_stream_t *s,
    char32_t const **x)
{
    va_read_iter_t iter = VA_READ_ITER(&va_char32_p_read_vtab_utf32, *x);
    (void)va_xprintf_iter(s, &iter);
    *x = iter.cur;
    return s;
}

extern va_stream_t *va_xprintf_char32_pp_utf32(
    va_stream_t *s,
    char32_t **x)
{
    return va_xprintf_char32_const_pp_utf32(s, (char32_t const **)x);
}

extern va_stream_t *va_xprintf_last_char32_p_utf32(
    va_stream_t *s,
    char32_t const *x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_char32_p_utf32(s,x);
}

extern va_stream_t *va_xprintf_last_char32_const_pp_utf32(
    va_stream_t *s,
    char32_t const **x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_char32_const_pp_utf32(s,x);
}

extern va_stream_t *va_xprintf_last_char32_pp_utf32(
    va_stream_t *s,
    char32_t **x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_char32_pp_utf32(s,x);
}

extern unsigned va_arr32_p_take_utf32(
    va_read_iter_t *iter_super,
    void const *end)
{
    assert(iter_super->cur != NULL);
    va_read_iter_end_t *iter= va_boxof(*iter, iter_super, super);
    if (iter_super->cur == iter->end) {
        return 0;
    }
    return va_char32_p_take_utf32(iter_super, end);
}

extern va_stream_t *va_xprintf_arr32_p_utf32(
    va_stream_t *s,
    va_arr32_t const *x)
{
    va_read_iter_end_t iter = {
        .super = VA_READ_ITER(&va_arr32_p_read_vtab_utf32, x->data),
        .end = x->data + x->size
    };
    return va_xprintf_iter(s, &iter.super);
}

extern va_stream_t *va_xprintf_last_arr32_p_utf32(
    va_stream_t *s,
    va_arr32_t const *x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_arr32_p_utf32(s,x);
}
