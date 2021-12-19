/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <stddef.h>
#include <assert.h>
#include <va_print/char.h>
#include <va_print/utf8.h>
#include <va_print/core.h>
#include <va_print/impl.h>

/* ********************************************************************** */
/* extern objects */

va_read_iter_vtab_t const va_char_p_read_vtab_utf8 = {
    "char*",
    va_char_p_take_utf8,
    va_char_p_end,
    0,
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
    unsigned char const *i = iter->cur;
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
    unsigned char const *s = i->cur;
    s += n;
    i->cur = s;
}

/* ********************************************************************** */
/* extern functions */

extern unsigned va_char_p_take_utf8(va_read_iter_t *iter, void const *end)
{
    assert(iter->cur != NULL);
    /* byte 0 */
    unsigned c0;
    (void)iter_nth(&c0, iter, end, 0);
    if (c0 == 0) {
        /* end of string */
        return 0;
    }
    if (c0 < 0x80) {
        iter_advance(iter, 1);
        return c0;
    }
    if (c0 <= 0xbf) {
        /* stray continuation bytes: mark with ECONT bit */
        iter_advance(iter, 1);
        return c0 | VA_U_ENC_UTF8 | VA_U_ECONT;
    }
    if (c0 < 0xc2) {
        /* 0xc0, 0xc1 are always illegal */
        goto error;
    }
    if (c0 > 0xf4) {
        /* 0xf5, .., 0xff are always illegal */
        goto error;
    }

    /* byte 1 */
    unsigned cx;
    if (!iter_nth(&cx, iter, end, 1)) {
        return 0;
    }
    if ((cx & 0xc0) != 0x80) {
        /* not a continuation byte */
        goto error;
    }

    /* detect errors as early as possible to get EMORE right */
    switch (c0) {
    /* overlong: 3 byte seq: min 0x800 = 0b0000_100000_000000 = E0 A0 80 */
    case 0xe0:
        if (cx < 0xA0) { goto error; }
        break;

    /* overlong: 4 byte seq: min 0x10000 = 0b000_010000_000000_000000 = F0 90 80 80 */
    case 0xf0:
        if (cx < 0x90) { goto error; }
        break;

    /* surrogates: 3 byte seq: min 0xd800 = 0b1101_100000_000000 = ED 90 80 */
    /* surrogates: 3 byte seq: max 0xdfff = 0b1101_111111_111111 = ED BF BF */
    /* abs max: 4 byte seq: max 0x10ffff = 0b100_001111_111111_111111 = F4 8F BF BF */
    case 0xed:
    case 0xf4:
        if (cx >= 0x90) { goto error; }
        break;
    }

    unsigned r = (c0 << 6) | (cx & 0x3f);
    if (c0 < 0xe0) {
        r &= 0x7ff;
        assert (r >= 0x80);
        iter_advance(iter, 2);
        return r;
    }

    /* byte 2 */
    if (!iter_nth(&cx, iter, end, 2)) {
        return 0;
    }
    if ((cx & 0xc0) != 0x80) {
        VA_BSET(c0, VA_U_EMORE, 1); /* first byte was OK */
        goto error;
    }
    r = (r << 6) | (cx & 0x3f);
    if (c0 < 0xf0) {
        r &= 0xffff;
        assert(r >= 0x0800);
        assert((r < VA_U_SURR_MIN) || (r > VA_U_SURR_MAX));
        iter_advance(iter, 3);
        return r;
    }

    /* byte 3 */
    if (!iter_nth(&cx, iter, end, 3)) {
        return 0;
    }
    if ((cx & 0xc0) != 0x80) {
        VA_BSET(c0, VA_U_EMORE, 2); /* first two bytes were OK */
        goto error;
    }
    r = (r << 6) | (cx & 0x3f);
    r &= 0x1fffff;
    assert(r >= 0x010000);
    assert(r <= 0x10ffff);
    iter_advance(iter, 4);
    return r;

error:
    iter_advance(iter, 1);
    return c0 | VA_U_ENC_UTF8;
}

extern void va_put_utf8(
    va_stream_t *s,
    unsigned c,
    void (*put)(va_stream_t *, char))
{
    /* decoder errors */
    if ((c & VA_U_ENC) != 0) {
        if ((c & VA_U_ENC) == VA_U_ENC_UTF8) {
            /* pass-through erroneous byte */
            put(s, (char)(c & 0xff));
            return;
        }
        /* skip continued errors after VA_U_REPLACEMENT */
        if ((c & VA_U_ECONT) != 0) {
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
    if (c <= 0x7f) {
        put(s, c & 0x7f);
        return;
    }
    if (c <= 0x7ff) {
        put(s, (char)(0xc0 | ((c >> 6) & 0x1f)));
        put(s, (char)(0x80 | ((c >> 0) & 0x3f)));
        return;
    }
    if (c <= 0xffff) {
        put(s, (char)(0xe0 | ((c >> 12) & 0x0f)));
        put(s, (char)(0x80 | ((c >> 6)  & 0x3f)));
        put(s, (char)(0x80 | ((c >> 0)  & 0x3f)));
        return;
    }

    put(s, (char)(0xf0 | ((c >> 18) & 0x07)));
    put(s, (char)(0x80 | ((c >> 12) & 0x3f)));
    put(s, (char)(0x80 | ((c >> 6)  & 0x3f)));
    put(s, (char)(0x80 | ((c >> 0)  & 0x3f)));
}

extern va_stream_t *va_xprintf_char_p_utf8(
    va_stream_t *s,
    char const *x)
{
    va_read_iter_t iter = VA_READ_ITER(&va_char_p_read_vtab_utf8, x);
    return va_xprintf_iter(s, &iter);
}

extern va_stream_t *va_xprintf_char_const_pp_utf8(
    va_stream_t *s,
    char const **x)
{
    va_read_iter_t iter = VA_READ_ITER(&va_char_p_read_vtab_utf8, *x);
    (void)va_xprintf_iter(s, &iter);
    *x = iter.cur;
    return s;
}

extern va_stream_t *va_xprintf_char_pp_utf8(
    va_stream_t *s,
    char **x)
{
    return va_xprintf_char_const_pp_utf8(s, (char const **)x);
}

extern va_stream_t *va_xprintf_last_char_p_utf8(
    va_stream_t *s,
    char const *x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_char_p_utf8(s, x);
}

extern va_stream_t *va_xprintf_last_char_const_pp_utf8(
    va_stream_t *s,
    char const **x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_char_const_pp_utf8(s, x);
}

extern va_stream_t *va_xprintf_last_char_pp_utf8(
    va_stream_t *s,
    char **x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_char_pp_utf8(s, x);
}
