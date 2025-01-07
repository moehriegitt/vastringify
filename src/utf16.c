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
    va_char16_p_set_chunk_mode,
    false,
    false,
    'u',
    {0}
};

va_read_iter_vtab_t const va_span16_p_read_vtab_utf16 = {
    "char16_t*",
    va_span16_p_take_utf16,
    va_char16_p_end,
    va_span16_p_set_chunk_mode,
    true,
    false,
    'u',
    {0}
};

va_read_iter_vtab_t const va_char16_p_read_vtab_utf16_chunk = {
    "char16_t*",
    va_char16_p_take_utf16,
    va_char16_p_end,
    NULL,
    false,
    true,
    'u',
    {0}
};

va_read_iter_vtab_t const va_span16_p_read_vtab_utf16_chunk = {
    "char16_t*",
    va_span16_p_take_utf16,
    va_char16_p_end,
    NULL,
    true,
    true,
    'u',
    {0}
};

extern void va_char16_p_set_chunk_mode(
    va_read_iter_t *iter)
{
    assert(iter->vtab == &va_char16_p_read_vtab_utf16);
    iter->vtab = &va_char16_p_read_vtab_utf16_chunk;
}

extern void va_span16_p_set_chunk_mode(
    va_read_iter_t *iter)
{
    assert(iter->vtab == &va_span16_p_read_vtab_utf16);
    iter->vtab = &va_span16_p_read_vtab_utf16_chunk;
}

/* ********************************************************************** */
/* static functions */

static bool iter_nth(
    unsigned *result,
    va_read_iter_t *iter,
    void const *e,
    unsigned n)
{
    char16_t const *i = iter->cur;
    i += n;
    if (i == e) {
        *result = VA_U_EOT;
        return false;
    }
    if ((*i == 0) && !iter->vtab->has_size) {
        *result = VA_U_EOT;
        return true;
    }
    *result = *i;
    return true;
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
    if (c0 == VA_U_EOT) {
        /* end of string */
        return VA_U_EOT;
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
        if (iter->vtab->chunk_mode) {
            // stop before incomplete sequence
            return VA_U_EOT;
        }
        else {
            // one word worked, so return it
            iter_advance(iter, 1);
            return c0 | VA_U_ENC_UTF16;
        }
    }
    if ((cx < 0xdc00) || (cx >= 0xe000)) {
        /* not a low surrogate */
        goto error;
    }

    iter_advance(iter, 2);
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
    (void)va_xprintf_iter_chunk(s, &iter);
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

extern unsigned va_span16_p_take_utf16(
    va_read_iter_t *iter_super,
    void const *end)
{
    assert(iter_super->cur != NULL);
    va_read_iter_end_t *iter= va_boxof(iter_super, *iter, super);
    if (iter_super->cur == iter->end) {
        return VA_U_EOT;
    }
#if 0
    /* avoid reading past the end of the span if the end is inside an UTF-8 char */
    if ((end == NULL) || (iter->end < end)) {
        end = iter->end;
    }
#endif
    return va_char16_p_take_utf16(iter_super, end);
}

extern va_stream_t *va_xprintf_span16_p_utf16(
    va_stream_t *s,
    va_span16_t const *x)
{
    va_read_iter_end_t iter = {
        .super = VA_READ_ITER(&va_span16_p_read_vtab_utf16, x->data),
        .end = x->data + x->size
    };
    return va_xprintf_iter(s, &iter.super);
}

extern va_stream_t *va_xprintf_last_span16_p_utf16(
    va_stream_t *s,
    va_span16_t const *x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_span16_p_utf16(s,x);
}
