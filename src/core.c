/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <assert.h>
#include <string.h>
#include "va_print/core.h"
#include "va_print/impl.h"

/* ********************************************************************** */
/* types */

typedef va_stream_t *(*render_int_t)(
    va_stream_t *s,
    unsigned long long x,
    unsigned base,
    char const **digit2,
    unsigned char prefix0);

/* ********************************************************************** */
/* static object definitions */

static char const *digit2_std[2] = {
    "0123456789abcdefghijklmnopqrstuvwxyz",
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
};
static char const *digit2_b32[2] = {
    "abcdefghijklmnopqrstuvwxyz234567____",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567____"
};

/* ********************************************************************** */
/* static functions */

static void render_dec(va_stream_t *s, unsigned c)
{
    if (c == 0) {
        return;
    }
    if (s->width > 0) {
        s->width--;
    }
}

static void render_putc(va_stream_t *s, unsigned c)
{
    if (c == 0) {
        return;
    }
    s->vtab->put(s, c);
}

static unsigned char va_quote_sh(unsigned c)
{
    if ((c >= 'a') && (c <= 'z')) { return 0; }
    if ((c >= 'A') && (c <= 'Z')) { return 0; }
    if ((c >= '0') && (c <= '9')) { return 0; }
    switch (c) {
    case '.': return 0;
    case '/': return 0;
    case ',': return 0;
    case '_': return 0;
    case '-': return 0;
    case '+': return 0;
    default:  return 1;
    }
}

static unsigned char va_quote_c(unsigned c)
{
    switch (c) {
    case '\b': return 'b';
    case '\f': return 'f';
    case '\n': return 'n';
    case '\r': return 'r';
    case '\t': return 't';
    case '\'': return '\'';
    case '\"': return '\"';
    case '\\': return '\\';
    default:   return 0;
    }
}

static void render_quote_octal(
    va_stream_t *s,
    unsigned c,
    void (*render)(va_stream_t *, unsigned))
{
    render(s, '\\');
    render(s, '0' + ((c >> 6) & 7));
    render(s, '0' + ((c >> 3) & 7));
    render(s, '0' + ((c >> 0) & 7));
}

static void render_quote_unicode(
    va_stream_t *s,
    unsigned c,
    void (*render)(va_stream_t *, unsigned))
{
    unsigned cnt = 0;
    render(s, '\\');
    if (c <= 0xffff) {
        render(s, 'u');
        cnt = 4;
    }
    else {
        render(s, 'U');
        cnt = 8;
    }
    char const *digit = digit2_std[!!(s->opt & VA_OPT_UPPER)];
    while (cnt > 0) {
        cnt--;
        render(s, (unsigned char)digit[(c >> (cnt * 4)) & 15]);
    }
}

static void iter_start(va_stream_t *s, va_read_iter_t *iter)
{
    VA_BSET(s->opt, VA_OPT_EMORE, 0);
    va_read_iter_start(iter);
}

static unsigned iter_take(va_stream_t *s, va_read_iter_t *iter)
{
    unsigned c = va_read_iter_take(iter);

    /* if no more continued errors are allowed, then clear ECONT flag
     * so that at least one REPLACEMENT_CHAR is output. */
    unsigned emore = VA_BGET(s->opt, VA_OPT_EMORE);
    if (emore == 0) {
        VA_MCLR(c, VA_U_ECONT);
    }
    else {
        if (c & VA_U_ECONT) {
            emore--;
        }
        else {
            emore = 0;
        }
        VA_BSET(s->opt, VA_OPT_EMORE, emore);
    }

    /* remember if an illegal char is printed, and how many may follow */
    if ((c & VA_U_ENC) != 0) {
        va_stream_set_error(s, VA_E_DECODE);
        if (emore == 0) {
            emore = VA_BGET(c, VA_U_EMORE);
            if (emore > 0) {
                VA_BSET(s->opt, VA_OPT_EMORE, emore);
            }
        }
    }

    return c;
}

static void render_quotec(
    va_stream_t *s,
    unsigned c,
    void (*render)(va_stream_t *, unsigned))
{
    unsigned c2;
    switch (s->opt & VA_OPT_QUOTE) {
    default:
        break;

    case VA_OPT_QUOTE_C:
    case VA_OPT_QUOTE_J:
        if (s->opt & VA_OPT_ZERO) {
            if (c & VA_U_ENC) {
                if (c & VA_U_ECONT) {
                    return;
                }
                c = VA_U_REPLACEMENT;
            }
            else
            if (!va_u_valid(c)) {
                va_stream_set_error(s, VA_E_ENCODE);
                c = VA_U_REPLACEMENT;
            }
        }

        c2 = va_quote_c(c);
        if (c2 > 0) {
            render(s, '\\');
            render(s, c2);
            return;
        }
        if ((c < 0x20) || (c == 0x7f)) {
            if ((s->opt & VA_OPT_QUOTE) == VA_OPT_QUOTE_C) {
                render_quote_octal(s, c, render);
            }
            else {
                render_quote_unicode(s, c, render);
            }
            return;
        }
        if (((s->opt & VA_OPT_ZERO) != 0) && (c >= 0x80)) {
            render_quote_unicode(s, c, render);
            return;
        }
        break;

    case VA_OPT_QUOTE_SH:
        if (c == '\'') {
            render(s, '\'');
            render(s, '\\');
            render(s, '\'');
            render(s, '\'');
            return;
        }
        break;
    }

    render(s, c);
}

static va_stream_t *render_int(
    va_stream_t *s,
    unsigned long long x,
    unsigned base,
    char const **digit2,
    unsigned char prefix0);

static va_stream_t *render_ptr(va_stream_t *s, void const *x)
{
    return render_int(s, (size_t)x, 16, digit2_std, 0);
}

static va_stream_t *render_iter(va_stream_t *s, va_read_iter_t *iter)
{
    if (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_PTR) {
        return render_ptr(s, iter->start);
    }

    if (iter->start == NULL) {
        va_stream_set_error(s, VA_E_NULL);
        return s;
    }

    /* compute string array max. size */
    if (s->opt & VA_OPT_PREC) {
        if (s->prec < iter->size) {
            iter->size = s->prec;
        }
    }

    /* quotation marks */
    unsigned ch;
    unsigned char delim = 0;
    if ((s->opt & VA_OPT_VAR) == 0) {
        switch (s->opt & VA_OPT_QUOTE) {
        default: break;
        case VA_OPT_QUOTE_C:
        case VA_OPT_QUOTE_J:
            delim = (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR) ? '\'' : '"';
            break;
        case VA_OPT_QUOTE_SH:
            for (iter_start(s,iter); (ch = iter_take(s,iter)) != 0;) {
                if (va_quote_sh(ch)) {
                    delim = '\'';
                    break;
                }
            }
            break;
        }
    }

    /* reinterpret 'width' into how many spaces are written */
    if (s->width <= (delim ? 2 : 0)) {
        s->width = 0;
    }
    else {
        s->width -= (delim ? 2 : 0);
        iter_start(s,iter);
        while ((s->width > 0) && ((ch = iter_take(s,iter)) != 0)) {
            render_quotec(s, ch, render_dec);
        }
    }

    /* space */
    if ((s->opt & VA_OPT_MINUS) == 0) {
        for (; s->width > 0; s->width--) {
            render_putc(s, ' ');
        }
    }

    /* meat */
    render_putc(s, delim);
    for (iter_start(s,iter); (ch = iter_take(s,iter)) != 0;) {
        render_quotec(s, ch, render_putc);
    }
    render_putc(s, delim);

    /* space */
    if ((s->opt & VA_OPT_MINUS) != 0) {
        for (; s->width > 0; s->width--) {
            render_putc(s, ' ');
        }
    }

    return s;
}

static va_stream_t *render_int_algo(
    va_stream_t *s,
    unsigned long long x,
    unsigned base,
    char const **digit2,
    unsigned char prefix0)
{
    if (VA_BGET(s->opt, VA_OPT_BASE) >= 2) {
        base = VA_BGET(s->opt, VA_OPT_BASE);
    }
    if (base > 36) {
        base = 36;
    }

    /* if we print a '-', then numerically negate */
    if (prefix0 == '-') {
        x = -x;
    }

    unsigned char prefix1 = 0;
    unsigned char prefix2 = 0;
    if ((s->opt & VA_OPT_VAR) != 0) {
        switch (base) {
        default:
            break;
        case 2:
            prefix1 = '0';
            prefix2 = (s->opt & VA_OPT_UPPER) ? 'B' : 'b';
            break;
        case 8:
            prefix1 = '0';
            break;
        case 16:
            prefix1 = '0';
            prefix2 = (s->opt & VA_OPT_UPPER) ? 'X' : 'x';
            break;
        case 32:
            prefix1 = '0';
            prefix2 = (s->opt & VA_OPT_UPPER) ? 'E' : 'e';
            break;
        }
    }

    /* compute length */
    size_t len = 0;
    unsigned long long div = 0;
    if ((x > 0) || (s->prec > 0)) {
        div = 1;
        len++;
        while ((x / div) >= base) {
            div *= base;
            len++;
        }
    }
    size_t blen = len;
    if (len < s->prec) {
        len = s->prec;
    }
    if (x > 0) {
        len += !!prefix1;
        len += !!prefix2;
    }
    len += !!prefix0;
    if (((s->opt & (VA_OPT_ZERO|VA_OPT_PREC|VA_OPT_MINUS)) == VA_OPT_ZERO) &&
        (len < s->width))
    {
        len = s->width;
    }

    size_t slen = (s->width > len) ? (s->width - len) : 0;

    /* print */
    if ((s->opt & VA_OPT_MINUS) == 0) {
        for (unsigned i = 0; i < slen; i++) {
            render_putc(s, ' ');
        }
    }
    if ((len > blen) && prefix0) {
        render_putc(s, prefix0);
        len--;
    }
    if ((len > blen) && prefix1) {
        render_putc(s, prefix1);
        len--;
    }
    if ((len > blen) && prefix2) {
        render_putc(s, prefix2);
        len--;
    }

    for (size_t i = blen; i < len; i++) {
        render_putc(s, '0');
    }
    char const *digit = digit2[!!(s->opt & VA_OPT_UPPER)];
    while (div > 0) {
        render_putc(s, (unsigned char)digit[(x / div) % base]);
        div /= base;
    }

    if ((s->opt & VA_OPT_MINUS) != 0) {
        for (unsigned i = 0; i < slen; i++) {
            render_putc(s, ' ');
        }
    }
    return s;
}

static unsigned take_utf64(va_read_iter_t *iter)
{
    if (iter->size == 0) {
        return 0;
    }
    if (iter->start != iter->cur) {
        return 0;
    }
    assert(iter->cur != NULL);
    assert(iter->size == 1);
    unsigned long long x = *(unsigned long long const *)iter->cur;
    iter->cur = NULL;

    unsigned u = x & VA_U_DATA;
    if (x > VA_U_MAXMAX) {
        return u | VA_U_ENC_ERR;
    }
    return u;
}

static va_stream_t *render_char(va_stream_t *s, unsigned long long x)
{
    return render_iter(s, &VA_READ_ITER(take_utf64, &x, 1));
}

static va_stream_t *render_int_b32(
    va_stream_t *s,
    unsigned long long x,
    unsigned base,
    char const **digit2,
    unsigned char prefix0)
{
    digit2 = digit2_b32;
    return render_int_algo(s, x, base, digit2, prefix0);
}

static va_stream_t *render_int_char(
    va_stream_t *s,
    unsigned long long x,
    unsigned base,
    char const **digit2,
    unsigned char prefix0)
{
    (void)base;
    (void)digit2;
    (void)prefix0;
    if ((s->opt & VA_OPT_SIZE) == VA_OPT_SIZE1) {
        x = (unsigned char)x;
    }
    if ((s->opt & VA_OPT_SIZE) == VA_OPT_SIZE2) {
        x = (unsigned short)x;
    }
    return render_char(s, x);
}

static const render_int_t render_int_select[] = {
    [VA_MODE_B32]  = render_int_b32,  /* = mode 1, base 32 */
    [VA_MODE_CHAR] = render_int_char, /* = mode 2 */
    [VA_MODE_PTR]  = render_int_algo, /* = mode 3 */
    [VA_MODE_NORM] = render_int_algo, /* = mode 0, */
};

static va_stream_t *render_int(
    va_stream_t *s,
    unsigned long long x,
    unsigned base,
    char const **digit2,
    unsigned char prefix0)
{
    unsigned mode = VA_BGET(s->opt, VA_OPT_MODE);
    assert(mode <= VA_MASK(VA_OPT_MODE));
    render_int_t pr = NULL;
    if (mode < va_countof(render_int_select)) {
        pr = render_int_select[mode];
    }
    if (pr == NULL) {
        pr = render_int_algo;
    }
    return pr(s, x, base, digit2, prefix0);
}

static va_stream_t *render_ull(va_stream_t *s, unsigned long long x)
{
    if ((s->opt & VA_OPT_SIZE) == VA_OPT_SIZE1) {
        x = (unsigned char)x;
    }
    if ((s->opt & VA_OPT_SIZE) == VA_OPT_SIZE2) {
        x = (unsigned short)x;
    }
    return render_int(s, x, 10, digit2_std, 0);
}

static va_stream_t *render_sll(va_stream_t *s, long long x)
{
    if ((s->opt & VA_OPT_SIZE) == VA_OPT_SIZE1) {
        x = (signed char)x;
    }
    if ((s->opt & VA_OPT_SIZE) == VA_OPT_SIZE2) {
        x = (short)x;
    }

    unsigned char prefix0 = 0;
    if (x < 0) {
        prefix0 = '-';
    }
    else if (s->opt & VA_OPT_PLUS) {
        prefix0 = '+';
    }
    else if (s->opt & VA_OPT_SPACE) {
        prefix0 = ' ';
    }
    return render_int(s, (unsigned long long)x, 10, digit2_std, prefix0);
}

/* xprints */

static unsigned iter_take_pat(va_stream_t *s, va_read_iter_t *iter)
{
    s->pat = iter->cur;
    return iter_take(s, iter);
}

/**
 * Returns non-0 iff the same value should be printed again. */
static bool render_format(va_stream_t *s)
{
    if (s->pat == NULL) {
        return 0;
    }

    va_read_iter_t iter[1] = { VA_READ_ITER(s->get, s->pat, ~(size_t)0) };
    iter_start(s, iter);

    unsigned c = iter_take(s, iter);
    if (c == 0) {
        return 0;
    }

    if ((s->opt & VA_OPT_STATE4) != 0) {
        /* The '*' was not provided an integer => clear bit and pretend
         * nothing happened. */
        s->opt ^= VA_OPT_STATE4;
    }

    if ((s->opt & VA_OPT_STATE) == 0) {
        s->width = 0;
        s->prec = 1;
        s->opt &= VA_OPT_RESET;
        if (c != '%') {
            return 0;
        }

        c = iter_take_pat(s, iter);

        /* parse options */
        for(;;) {
            switch (c) {
            default:
                goto end_of_options;
            case '#':
                s->opt |= VA_OPT_VAR;
                break;
            case '-':
                s->opt |= VA_OPT_MINUS;
                break;
            case '+':
                s->opt |= VA_OPT_PLUS;
                break;
            case ' ':
                s->opt |= VA_OPT_SPACE;
                break;
            case '0':
                s->opt |= VA_OPT_ZERO;
                break;
            case '=':
                s->opt |= VA_OPT_EQUAL;
                break;
            }
            c = iter_take_pat(s, iter);
        }
end_of_options:

        s->opt ^= VA_OPT_STATE1;

        /* parse width */
        if (c == '*') {
            s->pat = iter->cur;
            s->opt ^= VA_OPT_STATE4;
            return 0;
        }
        if ((c >= '1') && (c <= '9')) {
            s->width = c - '0';
            c = iter_take_pat(s, iter);
            while ((c >= '0') && (c <= '9')) {
                s->width = (s->width * 10) + c - '0';
                c = iter_take_pat(s, iter);
            }
        }
    }

    if ((s->opt & VA_OPT_STATE) == VA_OPT_STATE1) {
        s->opt ^= VA_OPT_STATE2;

        /* parse precision */
        if (c == '.') {
            s->opt |= VA_OPT_PREC;
            s->prec = 0;
            c = iter_take_pat(s, iter);
            if (c == '*') {
                s->pat = iter->cur;
                s->opt ^= VA_OPT_STATE4;
                return 0;
            }
            while ((c >= '0') && (c <= '9')) {
                s->prec = (s->prec * 10) + c - '0';
                c = iter_take_pat(s, iter);
            }
        }
    }

    if ((s->opt & VA_OPT_STATE) == VA_OPT_STATE3) {
        s->opt ^= VA_OPT_STATE3;

        /* parse sizes and quotation */
        switch (c) {
        case 'h':
            c = iter_take_pat(s, iter);
            if (c == 'h') {
                c = iter_take_pat(s, iter);
                s->opt |= VA_OPT_SIZE1;
            }
            else {
                s->opt |= VA_OPT_SIZE2;
            }
            break;

        case 'Q':
            c = iter_take_pat(s, iter);
            s->opt |= VA_OPT_QUOTE_J;
            break;

        case 'q':
            c = iter_take_pat(s, iter);
            s->opt |= VA_OPT_QUOTE_C;
            break;

        case 'k':
            c = iter_take_pat(s, iter);
            s->opt |= VA_OPT_QUOTE_SH;
            break;
        }

        /* parse format */
        switch (c) {
        default:
            break;
        case 'b': case 'B':
            VA_BSET(s->opt, VA_OPT_BASE, 2);
            break;
        case 'o': case 'O':
            VA_BSET(s->opt, VA_OPT_BASE, 8);
            break;
        case 'd': case 'D':
        case 'i': case 'I':
        case 'u': case 'U':
            VA_BSET(s->opt, VA_OPT_BASE, 10);
            break;
        case 'x': case 'X':
            VA_BSET(s->opt, VA_OPT_BASE, 16);
            break;
        case 'c': case 'C':
            VA_BSET(s->opt, VA_OPT_MODE, VA_MODE_CHAR);
            break;
        case 'p': case 'P':
            VA_BSET(s->opt, VA_OPT_MODE, VA_MODE_PTR);
            VA_BSET(s->opt, VA_OPT_BASE, 16);
            s->opt |= VA_OPT_VAR;
            break;
        case 'e': case 'E':
            VA_BSET(s->opt, VA_OPT_MODE, VA_MODE_B32);
            VA_BSET(s->opt, VA_OPT_BASE, 32);
            break;
        }

        if ((c >= 'a') && (c <= 'z')) {
            s->pat = iter->cur;
        }
        else
        if ((c >= 'A') && (c <= 'Z')) {
            s->opt |= VA_OPT_UPPER;
            s->pat = iter->cur;
        }
    }

    assert((s->opt & VA_OPT_STATE) == 0);
    return s->opt & VA_OPT_EQUAL ? 1 : 0;
}

static void va_xprintf_skip(va_stream_t *s)
{
    if (s->pat == NULL) {
        return;
    }

    va_read_iter_t iter[1] = { VA_READ_ITER(s->get, s->pat, ~(size_t)0) };
    iter_start(s, iter);

    unsigned ch;
    while ((ch = iter_take(s, iter)) != 0) {
        if (ch == '%') {
            ch = iter_take(s, iter);
            if (ch != '%') {
                return;
            }
        }
        render_putc(s, ch);
        s->pat = iter->cur;
    }
}

static bool va_xprintf_set_ast(va_stream_t *s, long long x)
{
    if ((s->opt & VA_OPT_STATE) == VA_OPT_STATE5) {
        s->opt ^= VA_OPT_STATE4;
        if (x < 0) {
            s->opt |= VA_OPT_MINUS;
            x = -x;
        }
        s->width = (size_t)x;
        return 0;
    }

    if ((s->opt & VA_OPT_STATE) == VA_OPT_STATE7) {
        s->opt ^= VA_OPT_STATE4;
        s->prec = (unsigned)x;
        return 0;
    }

    return 1;
}

/* ********************************************************************** */
/* extern functions */

extern va_stream_t *va_xprintf_init(
    va_stream_t *s,
    void const *x,
    unsigned (*get)(va_read_iter_t *))
{
    /* init */
    s->pat = x;
    s->get = get;
    if (s->vtab->init != NULL) {
        s->vtab->init(s);
    }

    /* prefix part of the format string */
    va_xprintf_skip(s);

    /* first % format */
    render_format(s);
    return s;
}

extern va_stream_t *va_xprintf_iter(va_stream_t *s, va_read_iter_t *x)
{
    for(;;) {
        va_read_iter_t xi = *x;
        if (va_xprintf_set_ast(s, 0)) {
            render_iter(s, &xi);
            va_xprintf_skip(s);
        }
        if (!render_format(s)) {
            x->cur = xi.cur;
            return s;
        }
    }
}

extern va_stream_t *va_xprintf_ull(va_stream_t *s, unsigned long long x)
{
    do {
        if (va_xprintf_set_ast(s, (long long)x)) {
            render_ull(s, x);
            va_xprintf_skip(s);
        }
    } while (render_format(s));
    return s;
}

extern va_stream_t *va_xprintf_sll(va_stream_t *s, long long x)
{
    do {
        if (va_xprintf_set_ast(s, x)) {
            render_sll(s, x);
            va_xprintf_skip(s);
        }
    } while (render_format(s));
    return s;
}

extern va_stream_t *va_xprintf_ptr(va_stream_t *s, void const *x)
{
    do {
        if (va_xprintf_set_ast(s, 0)) {
            render_ptr(s, x);
            va_xprintf_skip(s);
        }
    } while (render_format(s));
    return s;
}

extern va_stream_t *va_xprintf_short(va_stream_t *s, short x)
{
    do {
        if (va_xprintf_set_ast(s, x)) {
            s->opt |= VA_OPT_SIZE2;
            render_sll(s, x);
            va_xprintf_skip(s);
        }
    } while (render_format(s));
    return s;
}

extern va_stream_t *va_xprintf_schar(va_stream_t *s, signed char x)
{
    do {
        if (va_xprintf_set_ast(s, x)) {
            s->opt |= VA_OPT_SIZE1;
            render_sll(s, x);
            va_xprintf_skip(s);
        }
    } while (render_format(s));
    return s;
}

extern va_stream_t *va_xprintf_char(va_stream_t *s, char x)
{
    if ((char)-1 < 0) {
        return va_xprintf_schar(s, (signed char)x);
    }
    else {
        return va_xprintf_ull(s, (unsigned char)x);
    }
}

extern va_stream_t *va_xprintf_error_t_p(va_stream_t *s, va_error_t *x)
{
    x->code = VA_BGET(s->opt, VA_OPT_ERR);
    VA_BSET(s->opt, VA_OPT_ERR, 0);
    return s;
}
