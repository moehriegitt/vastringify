/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <stddef.h>
#include <assert.h>
#include <va_print/char.h>
#include <va_print/utf16.h>
#include <va_print/core.h>
#include <va_print/impl.h>

/* ********************************************************************** */
/* extern objects */

va_read_iter_vtab_t const va_char16_p_read_vtab_utf16 = {
    "char16_t*",
    va_char16_p_take_utf16,
    va_char16_p_end,
    'u',
    {0}
};

va_read_iter_vtab_t const va_arr16_p_read_vtab_utf16 = {
    "char16_t*",
    va_arr16_p_take_utf16,
    va_char16_p_end,
    'U',
    {0}
};

/* ********************************************************************** */
/* static functions */

static int iter_nth(
    unsigned *res,
    va_read_iter_t *iter,
    void const *e,
    unsigned n)
{
    char16_t const *i = iter->cur;
    i += n;

    if (i == e) {
        *res = 0;
        return 0;
    }

    *res = *i;
    return 1;
}

static void iter_advance(
    va_read_iter_t *i,
    unsigned n)
{
    char16_t const *s = i->cur;
    s += n;
    i->cur = s;
}

/* ********************************************************************** */
/* extern functions */

extern unsigned va_char16_p_take_utf16(
    va_read_iter_t *iter,
    void const *end)
{
    assert(iter->cur != NULL);
    /* word 0 */
    unsigned c0;
    (void)iter_nth(&c0, iter, end, 0);
    if (c0 == 0) {
        /* end of string */
        return 0;
    }
    if ((c0 < 0xd800) || (c0 >= 0xe000)) {
        iter_advance(iter, 1);
        return c0;
    }
    if (c0 >= 0xdc00) {
        /* stray low surrogate */
        goto error;
    }

    unsigned cx;
    if (!iter_nth(&cx, iter, end, 1)) {
        return 0;
    }
    if ((cx < 0xdc00) || (cx >= 0xe000)) {
        /* not a low surrogate */
        goto error;
    }

    c0 &= 0x3ff;
    cx &= 0x3ff;
    return (c0 << 10) + cx + 0x10000;

error:
    iter_advance(iter, 1);
    return c0 | VA_U_ENC_UTF16;
}

extern void va_put_utf16(
    va_stream_t *s,
    unsigned c,
    void (*put)(va_stream_t *, char16_t))
{
    /* decoder errors */
    if ((c & VA_U_ENC) != 0) {
        if ((c & VA_U_ENC) == VA_U_ENC_UTF16) {
            /* pass-through erroneous word */
            put(s, c & 0xffff);
            return;
        }
        c = VA_U_REPLACEMENT;
    }

    /* encoding errors */
    if (!va_u_valid(c)) {
        va_stream_set_error(s, VA_E_ENCODE);
        c = VA_U_REPLACEMENT;
    }

    /* now do the encoding */
    if (c <= 0xffff) {
        put(s, c & 0xffff);
        return;
    }

    c -= 0x10000;
    put(s, (char16_t)(0xd800 + (c >> 10)));
    put(s, (char16_t)(0xdc00 + (c & 0x3ff)));
}

extern va_stream_t *va_xprintf_char16_p_utf16(
    va_stream_t *s,
    char16_t const *x)
{
    va_read_iter_t iter = VA_READ_ITER(&va_char16_p_read_vtab_utf16, x);
    return va_xprintf_iter(s, &iter);
}

extern va_stream_t *va_xprintf_char16_const_pp_utf16(
    va_stream_t *s,
    char16_t const **x)
{
    va_read_iter_t iter = VA_READ_ITER(&va_char16_p_read_vtab_utf16, *x);
    (void)va_xprintf_iter(s, &iter);
    *x = iter.cur;
    return s;
}

extern va_stream_t *va_xprintf_char16_pp_utf16(
    va_stream_t *s,
    char16_t **x)
{
    return va_xprintf_char16_const_pp_utf16(s, (char16_t const **)x);
}

extern va_stream_t *va_xprintf_last_char16_p_utf16(
    va_stream_t *s,
    char16_t const *x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_char16_p_utf16(s,x);
}

extern va_stream_t *va_xprintf_last_char16_const_pp_utf16(
    va_stream_t *s,
    char16_t const **x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_char16_const_pp_utf16(s,x);
}

extern va_stream_t *va_xprintf_last_char16_pp_utf16(
    va_stream_t *s,
    char16_t **x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_char16_pp_utf16(s,x);
}

extern unsigned va_arr16_p_take_utf16(
    va_read_iter_t *iter_super,
    void const *end)
{
    assert(iter_super->cur != NULL);
    va_read_iter_end_t *iter= va_boxof(*iter, iter_super, super);
    if (iter_super->cur == iter->end) {
        return 0;
    }
    return va_char16_p_take_utf16(iter_super, end);
}

extern va_stream_t *va_xprintf_arr16_p_utf16(
    va_stream_t *s,
    va_arr16_t const *x)
{
    va_read_iter_end_t iter = {
        .super = VA_READ_ITER(&va_arr16_p_read_vtab_utf16, x->data),
        .end = x->data + x->size
    };
    return va_xprintf_iter(s, &iter.super);
}

extern va_stream_t *va_xprintf_last_arr16_p_utf16(
    va_stream_t *s,
    va_arr16_t const *x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_arr16_p_utf16(s,x);
}
