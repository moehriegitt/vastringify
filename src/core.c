/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

#include <assert.h>
#include <string.h>
#include "va_print/core.h"
#include "va_print/impl.h"

/* ********************************************************************** */
/* sigil to use in format strings */
#ifndef VA_SIGIL
#define VA_SIGIL '~'
#endif

/* ********************************************************************** */
/* static object definitions */

static char const *digit2_std[2] = {
    "0123456789abcdefghijklmnopqrstuvwxyz",
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
};
static char const *digit2_b32[2] = {
    "abcdefghijklmnopqrstuvwxyz234567",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"
};

/* ********************************************************************** */
/* static functions */

#define render va_stream_render

extern void va_stream_render(va_stream_t *s, unsigned c)
{
    if (c == 0) {
        return;
    }
    if (s->width > 0) {
        s->width--;
    }
    if ((s->opt & VA_OPT_SIM) == 0) {
        s->vtab->put(s, c);
    }
}

static bool check_quote_sh(va_stream_t *s, unsigned c)
{
    s->qctxt = 1; /* the string is not empty */
    if ((c >= 'a') && (c <= 'z')) { return false; }
    if ((c >= 'A') && (c <= 'Z')) { return false; }
    if ((c >= '0') && (c <= '9')) { return false; }
    switch (c) {
    case '.': return false;
    case '/': return false;
    case ',': return false;
    case '_': return false;
    case '-': return false;
    case '+': return false;
    default:  return true;
    }
}

static bool check_flush_sh(va_stream_t *s)
{
    return (s->qctxt == 0); /* quote if the string empty */
}

static unsigned char va_quote1_c(unsigned c)
{
    switch (c) {
    case '\n': return 'n';
    case '\r': return 'r';
    case '\t': return 't';
    case '\'': return '\'';
    case '\"': return '\"';
    case '\\': return '\\';
    default:   return 0;
    }
}

extern void va_stream_render_quote_oct(
    va_stream_t *s,
    unsigned c)
{
    render(s, '\\');
    render(s, '0' + ((c >> 6) & 7));
    render(s, '0' + ((c >> 3) & 7));
    render(s, '0' + ((c >> 0) & 7));
}

extern void va_stream_render_quote_u(
    va_stream_t *s,
    unsigned c)
{
    char const * const digit = digit2_std[!!(s->opt & VA_OPT_UPPER)];
    render(s, '\\');
    if (c <= 0xffff) {
        render(s, 'u');
    }
    else {
        render(s, 'U');
        render(s, (unsigned char)digit[(c >> 28) & 15]);
        render(s, (unsigned char)digit[(c >> 24) & 15]);
        render(s, (unsigned char)digit[(c >> 20) & 15]);
        render(s, (unsigned char)digit[(c >> 16) & 15]);
    }
    render(s, (unsigned char)digit[(c >> 12) & 15]);
    render(s, (unsigned char)digit[(c >>  8) & 15]);
    render(s, (unsigned char)digit[(c >>  4) & 15]);
    render(s, (unsigned char)digit[(c >>  0) & 15]);
}

static void iter_start(
    va_stream_t *s,
    va_read_iter_t *iter,
    void const *start)
{
    VA_BSET(s->opt, VA_OPT_EMORE, 0);
    s->qctxt = 0;
    iter->cur = start;
}

static unsigned iter_take(
    va_stream_t *s,
    va_read_iter_t *iter,
    void const *end)
{
    if (iter->cur == NULL) {
        va_stream_set_error(s, VA_E_NULL);
        return VA_U_EOT;
    }

    unsigned c = iter->vtab->take(iter, end);
    if (c == VA_U_EOT) {
        return c;
    }

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

static void render_quote_c(
    va_stream_t *s,
    unsigned c)
{
    if (s->opt & VA_OPT_ZERO) {
        if (c & VA_U_ENC) {
            if (c & VA_U_ECONT) {
                return;
            }
            c = VA_U_REPLACEMENT;
        }
        else if (!va_u_valid(c)) {
            va_stream_set_error(s, VA_E_ENCODE);
            c = VA_U_REPLACEMENT;
        }
    }

    unsigned c2 = va_quote1_c(c);
    if (c2 > 0) {
        render(s, '\\');
        render(s, c2);
        return;
    }
    if ((c < 0x20) || (c == 0x7f)) {
        va_stream_render_quote_oct(s, c);
        return;
    }
    if (((s->opt & VA_OPT_ZERO) != 0) && (c >= 0x80)) {
        va_stream_render_quote_u(s, c);
        return;
    }

    va_stream_render(s, c);
}

static va_quotation_t quote_c = {
    .delim = { VA_DELIM(0xff, '\'', '\''), VA_DELIM(0xff, '"', '"') },
    .check_quote = NULL,
    .render_quote = render_quote_c,
};

static void render_quote_j(
    va_stream_t *s,
    unsigned c)
{
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

    unsigned c2 = va_quote1_c(c);
    if (c2 > 0) {
        render(s, '\\');
        render(s, c2);
        return;
    }
    if ((c < 0x20) || (c == 0x7f)) {
        va_stream_render_quote_u(s, c);
        return;
    }
    if (((s->opt & VA_OPT_ZERO) != 0) && (c >= 0x80)) {
        va_stream_render_quote_u(s, c);
        return;
    }
    va_stream_render(s, c);
}

static va_quotation_t quote_j = {
    .delim = { VA_DELIM(0, '\'', '\''), VA_DELIM(0, '"', '"') },
    .check_quote = NULL,
    .render_quote = render_quote_j,
};

static void render_quote_sh(
    va_stream_t *s,
    unsigned c)
{
    if (c == '\'') {
        render(s, '\'');
        render(s, '\\');
        render(s, '\'');
        render(s, '\'');
        return;
    }
    va_stream_render(s, c);
}

static va_quotation_t quote_sh = {
    .delim = { VA_DELIM(0, '\'', '\''), VA_DELIM(0, '\'', '\'') },
    .check_quote = check_quote_sh,
    .check_flush = check_flush_sh,
    .render_quote = render_quote_sh,
};

static va_quotation_t const *va_quote[VA_MASK(VA_OPT_QUOTE) + 1] = {
    [VA_QUOTE_q] = &quote_c,
    [VA_QUOTE_Q] = &quote_j,
    [VA_QUOTE_k] = &quote_sh,
};

extern va_quotation_t const *va_quotation_set(
    unsigned which,
    va_quotation_t const *quotation)
{
    which &= VA_MASK(VA_OPT_QUOTE);
    va_quotation_t const *old = va_quote[which];
    va_quote[which] = quotation;
    return old;
}

static void render_quote_put(
    va_stream_t *s,
    unsigned c)
{
    va_quotation_t const *q = va_quote[VA_BGET(s->opt, VA_OPT_QUOTE)];
    if (q != NULL) {
        VA_POSSIBLE_CALL("render_quote_c");
        VA_POSSIBLE_CALL("render_quote_j");
        VA_POSSIBLE_CALL("render_quote_sh");
        return q->render_quote(s, c);
    }
    va_stream_render(s, c);
}

static void render_quote_flush(
    va_stream_t *s)
{
    va_quotation_t const *q = va_quote[VA_BGET(s->opt, VA_OPT_QUOTE)];
    if ((q != NULL) && (q->render_flush != NULL)) {
        return q->render_flush(s);
    }
}

static void render_int(
    va_stream_t *s,
    unsigned long long x,
    unsigned base,
    unsigned char prefix0);

static void render_rawstr(va_stream_t *s, char const *start)
{
    assert(start != NULL);

    if ((s->opt & VA_OPT_MINUS) == 0) {
        /* reinterpret 'width' into how many spaces are written */
        for(char const *cur = start; (s->width > 0) && (*cur != 0); cur++) {
            s->width--;
        }

        /* space */
        while (s->width > 0) {
            render(s, ' ');
        }
    }

    /* meat */
    for(char const *cur = start; (*cur != 0); cur++) {
        render(s, (unsigned char)*cur);
    }

    /* space */
    while (s->width > 0) {
        render(s, ' ');
    }
}

static void render_ptr(va_stream_t *s, void const *x)
{
    if (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_TYPE) {
        render_rawstr(s, "void*");
        return;
    }
    if (VA_BGET(s->opt, VA_OPT_QUOTE) != 0) {
        s->opt |= VA_OPT_VAR;
    }
    render_int(s, (size_t)x, 16, 0);
    return;
}

static void render_iter_algo(va_stream_t *s, va_read_iter_t *iter)
{
    if (iter->cur == NULL) {
        switch (VA_BGET(s->opt, VA_OPT_QUOTE)) {
        case VA_QUOTE_q:
            render_rawstr(s, "NULL");
            return;

        case VA_QUOTE_K:
        case VA_QUOTE_Q:
            render_rawstr(s, "null");
            return;
        }
    }

    /* compute string array max. size */
    void const *start = iter->cur;
    void const *end = NULL;
    if (s->prec != VA_PREC_NONE) {
        end = iter->vtab->end(iter, s->prec);
    }

    /* quotation marks */
    unsigned ch;
    unsigned long long delim = 0;
    if ((s->opt & VA_OPT_VAR) == 0) {
        va_quotation_t const *q = va_quote[VA_BGET(s->opt, VA_OPT_QUOTE)];
        if (q != NULL) {
            if (q->check_quote == NULL) {
                delim = q->delim[VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR ? 0 : 1];
            }
            else {
                for (iter_start(s,iter,start); (ch = iter_take(s,iter,end)) != VA_U_EOT;) {
                    VA_POSSIBLE_CALL("check_quote_sh");
                    if (q->check_quote(s, ch)) {
                        delim |= q->delim[VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR ? 0 : 1];
                        goto done_check_quote;
                    }
                }
                if (start != NULL) {
                    VA_POSSIBLE_CALL("check_flush_sh");
                    if (q->check_flush && q->check_flush(s)) {
                        delim |= q->delim[VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR ? 0 : 1];
                    }
                }
            done_check_quote:;
            }
        }
    }
    if (VA_DELIM_PREFIX(delim) == 0xff) {
        delim ^= VA_DELIM(0xff, 0, 0);
        if ((VA_BGET(s->opt, VA_OPT_SIGN) == VA_SIGN_ZEXT) &&
            (iter->vtab->str_prefix))
        {
            delim += VA_DELIM(iter->vtab->str_prefix, 0, 0);
        }
    }
    delim += 0U + !!VA_DELIM_FRONT(delim) + !!VA_DELIM_BACK(delim) + !!VA_DELIM_PREFIX(delim);

    /* reinterpret 'width' into how many spaces are written */
    if ((s->opt & VA_OPT_MINUS) == 0) {
        if (s->width <= VA_DELIM_WIDTH(delim)) {
            s->width = 0;
        }
        else {
            s->width -= VA_DELIM_WIDTH(delim);
            s->opt |= VA_OPT_SIM;
            iter_start(s,iter,start);
            while ((s->width > 0) && ((ch = iter_take(s,iter,end)) != VA_U_EOT)) {
                render_quote_put(s, ch);
            }
            render_quote_flush(s);
            VA_MCLR(s->opt, VA_OPT_SIM);

            /* space */
            while (s->width > 0) {
                render(s, ' ');
            }
        }
    }

    /* meat */
    render(s, VA_DELIM_PREFIX(delim));
    render(s, VA_DELIM_FRONT(delim));
    for (iter_start(s,iter,start); (ch = iter_take(s,iter,end)) != VA_U_EOT;) {
        render_quote_put(s, ch);
    }
    render_quote_flush(s);
    render(s, VA_DELIM_BACK(delim));

    /* space */
    while (s->width > 0) {
        render(s, ' ');
    }
}

static void render_iter(va_stream_t *s, va_read_iter_t *iter, void const *start)
{
    iter->cur = start;
    switch (VA_BGET(s->opt, VA_OPT_MODE)) {
    case VA_MODE_TYPE:
        render_rawstr(s, iter->vtab->type);
        return;
    case VA_MODE_PTR:
        render_ptr(s, iter->cur);
        return;
    }
    render_iter_algo(s, iter);
}

typedef struct {
    va_stream_t s;
    va_stream_t *dst;
    size_t info;
    va_quotation_t const *q;
} stream_redirect_t;

static void custom_needquote(va_stream_t *s_, unsigned c)
{
    stream_redirect_t *s = (stream_redirect_t*)s_;
    if (s->q->check_quote(s->dst, c)) {
        s->info = 1;
    }
}

static void custom_put(va_stream_t *s_, unsigned c)
{
    stream_redirect_t *s = (stream_redirect_t*)s_;
    render_quote_put(s->dst, c);
}

static va_stream_vtab_t custom_needquote_vtab = {
    .put = custom_needquote,
};

static va_stream_vtab_t custom_put_vtab = {
    .put = custom_put,
};

static void render_custom(va_stream_t *s, va_print_t *print)
{
    stream_redirect_t s2 = { VA_STREAM(NULL), s, 0, 0 };
    print->width = s->width;
    print->prec = s->prec;
    print->opt = s->opt;

    assert(print->print != NULL);

    /* quotation marks */
    unsigned long long delim = 0;
    if ((s->opt & VA_OPT_VAR) == 0) {
        s2.q = va_quote[VA_BGET(s->opt, VA_OPT_QUOTE)];
        if (s2.q != NULL) {
            if (s2.q->check_quote == NULL) {
                delim = s2.q->delim[VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR ? 0 : 1];
            }
            else {
                s2.s.vtab = &custom_needquote_vtab;
                s2.info = 0;
                print->print(&s2.s, print);
                if ((s2.info != 0) || (s2.q->check_flush && s2.q->check_flush(s))) {
                    delim |= s2.q->delim[VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR ? 0 : 1];
                }
            }
        }
    }

    /* prefix is not handled here: the input is not an 'iter' with a size */
    delim += 0U + !!VA_DELIM_BACK(delim) + !!VA_DELIM_PREFIX(delim);

    s2.s.vtab = &custom_put_vtab;

    /* reinterpret 'width' into how many spaces are written */
    if ((s->opt & VA_OPT_MINUS) == 0) {
        if (s->width <= VA_DELIM_WIDTH(delim)) {
            s->width = 0;
        }
        else {
            s->width -= VA_DELIM_WIDTH(delim);
            s->opt |= VA_OPT_SIM;
            print->width = s->width;
            print->print(&s2.s, print);
            render_quote_flush(s);
            VA_MCLR(s->opt, VA_OPT_SIM);

            /* space */
            while (s->width > 0) {
                render(s, ' ');
            }
        }
    }

    /* meat */
    render(s, VA_DELIM_FRONT(delim));
    print->width = s->width;
    print->print(&s2.s, print);
    render_quote_flush(s);
    render(s, VA_DELIM_BACK(delim));

    /* space */
    while (s->width > 0) {
        render(s, ' ');
    }
}

static inline unsigned get_prec(va_stream_t *s, unsigned def)
{
    return s->prec == VA_PREC_NONE ? def : s->prec;
}

static void render_int(
    va_stream_t *s,
    unsigned long long x,
    unsigned base,
    unsigned char prefix0)
{
    /* base */
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
    if (x > 0) {
        if ((s->opt & VA_OPT_VAR) != 0) {
            switch (base) {
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
    }

    /* compute length */
    unsigned len = 0;
    unsigned long long div = 0;
    if ((x > 0) || (get_prec(s,1) > 0)) {
        div = 1;
        len++;
        while ((x / div) >= base) {
            div *= base;
            len++;
        }
    }
    unsigned blen = len;
    if (len < get_prec(s,1)) {
        len = get_prec(s,1);
    }

    len += !!prefix0;
    len += !!prefix1;
    len += !!prefix2;

    if (((s->opt & (VA_OPT_ZERO|VA_OPT_MINUS)) == VA_OPT_ZERO) &&
        (s->prec == VA_PREC_NONE) &&
        (len < s->width))
    {
        len = s->width;
    }

    if ((s->opt & VA_OPT_MINUS) == 0) {
        s->width = (s->width > len) ? (s->width - len) : 0;
        while (s->width > 0) {
            render(s, ' ');
        }
    }

    if (prefix0) {
        render(s, prefix0);
        len--;
    }
    if (prefix1) {
        render(s, prefix1);
        len--;
    }
    if (prefix2) {
        render(s, prefix2);
        len--;
    }

    while (len > blen) {
        render(s, '0');
        len--;
    }

    /* which set of digits? */
    char const **digit2 = digit2_std;
    if ((base <= 32) && (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_B32)) {
        digit2 = digit2_b32;
    }
    char const *digit = digit2[!!(s->opt & VA_OPT_UPPER)];
    while (div > 0) {
        render(s, (unsigned char)digit[(x / div) % base]);
        div /= base;
    }

    while (s->width > 0) {
        render(s, ' ');
    }
}

static unsigned arr1_utf64_take(va_read_iter_t *iter, void const *end)
{
    if ((iter->cur == end) || (iter->cur == NULL)) {
        return VA_U_EOT;
    }
    unsigned long long x = *(unsigned long long const *)iter->cur;
    iter->cur = NULL;

    unsigned u = x & VA_U_DATA;
    if (x > VA_U_MAXMAX) {
        return u | VA_U_ENC_ERR;
    }
    return u;
}

static void const *arr1_utf64_end(va_read_iter_t *iter, size_t size)
{
    if (size == 0) {
        return iter->cur;
    }
    return NULL;
}

static va_read_iter_vtab_t const arr1_vtab_8 = {
    NULL,
    arr1_utf64_take,
    arr1_utf64_end,
    true,
    0,
    {0},
};

static va_read_iter_vtab_t const arr1_vtab_16 = {
    NULL,
    arr1_utf64_take,
    arr1_utf64_end,
    true,
    'u',
    {0},
};

static va_read_iter_vtab_t const arr1_vtab_32 = {
    NULL,
    arr1_utf64_take,
    arr1_utf64_end,
    true,
    'U',
    {0},
};

static void render_char(
    va_stream_t *s,
    unsigned long long x,
    unsigned sz)
{
    static va_read_iter_vtab_t const * const vtab[8] = {
        [0] = &arr1_vtab_8,
        [1] = &arr1_vtab_16,
        [3] = &arr1_vtab_32,
        [7] = &arr1_vtab_32,
    };
    render_iter_algo(s, &VA_READ_ITER(vtab[(sz - 1) & 7], &x));
}

static void render_ull(va_stream_t *s, unsigned long long x, unsigned sz)
{
    static char const * const type_name[8] = {
        [0] = "uint8_t",
        [1] = "uint16_t",
        [3] = "uint32_t",
        [7] = "uint64_t",
    };
    if (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_TYPE) {
        render_rawstr(s, type_name[(sz - 1) & 7]);
        return;
    }

    switch (VA_BGET(s->opt, VA_OPT_SIZE)) {
    case 1: x = (unsigned short)x; break;
    case 2: x = (unsigned char)x; break;
    }

    if (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR) {
        render_char(s, x, sz);
        return;
    }
    render_int(s, x, 10, 0);
}

static void render_bool(va_stream_t *s, unsigned long long x)
{
    if (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_TYPE) {
        render_rawstr(s, "bool");
        return;
    }
    if (VA_BGET(s->opt, VA_OPT_BASE) >= 2) {
        render_int(s, x, 10, 0);
        return;
    }
    render_rawstr(s, x ? "true" : "false");
}

static void render_sll(va_stream_t *s, long long x, unsigned sz)
{
    static char const * const type_name[8] = {
        [0] = "int8_t",
        [1] = "int16_t",
        [3] = "int32_t",
        [7] = "int64_t",
    };
    if (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_TYPE) {
        render_rawstr(s, type_name[(sz - 1) & 7]);
        return;
    }

    switch (VA_BGET(s->opt, VA_OPT_SIZE)) {
    case 2:
        x = (signed char)x;
        sz = 1;
        break;
    case 1:
        x = (short)x;
        sz = 2;
        break;
    }

    unsigned long long ux = (unsigned long long)x;
    unsigned char prefix0 = 0;
    if (VA_BGET(s->opt, VA_OPT_SIGN) == VA_SIGN_ZEXT) {
        ux &= ~0ULL >> ((sizeof(long long) - sz) * 8);
    }
    else if (x < 0) {
        prefix0 = '-';
    }
    else if (VA_BGET(s->opt, VA_OPT_SIGN) == VA_SIGN_PLUS) {
        prefix0 = '+';
    }
    else if (VA_BGET(s->opt, VA_OPT_SIGN) == VA_SIGN_SPACE) {
        prefix0 = ' ';
    }

    if (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR) {
        ux &= ~0ULL >> ((sizeof(long long) - sz) *8);
        render_char(s, ux, sz);
        return;
    }

    render_int(s, ux, 10, prefix0);
}

/* xprints */

static unsigned iter_take_pat(va_stream_t *s, va_read_iter_t *iter)
{
    s->pat.cur = iter->cur;
    return iter_take(s, iter, NULL);
}

/**
 * Returns non-0 iff the same value should be printed again. */
static unsigned parse_format(va_stream_t *s)
{
    va_read_iter_t iter[1] = { s->pat };

again:;
    unsigned c = iter_take(s, iter, NULL);
    /* stay at end of string, regardless of STATE */
    if ((c == 0) || (c == VA_U_EOT)) {
        /* the format iterator always stops at NUL, too */
        goto end_of_format;
    }

    if (VA_BGET(s->opt, VA_OPT_STATE) <= VA_STATE_ARG) {
        s->width = 0;
        s->prec = VA_PREC_NONE;
        s->opt &= VA_OPT_RESET_ARG;

        /* print part between format specifiers */
        for (;;) {
            if ((c == 0) || (c == VA_U_EOT)) {
                goto end_of_format;
            }
            if (c == VA_SIGIL) {
                c = iter_take_pat(s, iter);
                break;
            }
            render(s, c);
            c = iter_take_pat(s, iter);
        }

        /* assume default width */
        s->width = VA_WIDTH_NONE;

        /* parse options */
        for(;;) {
            switch (c) {
            default:
                goto end_of_flags;
            case '#':
                s->opt |= VA_OPT_VAR;
                break;
            case '-':
                s->opt |= VA_OPT_MINUS;
                break;
            case '+':
                VA_BSET(s->opt, VA_OPT_SIGN, VA_SIGN_PLUS);
                break;
            case ' ':
                VA_BSET(s->opt, VA_OPT_SIGN, VA_SIGN_SPACE);
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
end_of_flags:

        VA_BSET(s->opt, VA_OPT_STATE, VA_STATE_WIDTH);

        /* parse width */
        if (c == '*') {
            s->width = 0;
            s->pat = *iter;
            goto ast_arg;
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

    if (VA_BGET(s->opt, VA_OPT_STATE) == VA_STATE_WIDTH) {
        VA_BSET(s->opt, VA_OPT_STATE, VA_STATE_PREC);

        /* parse precision */
        if (c == '.') {
            s->prec = 0;
            c = iter_take_pat(s, iter);
            if (c == '*') {
                s->pat = *iter;
                goto ast_arg;
            }
            while ((c >= '0') && (c <= '9')) {
                s->prec = (s->prec * 10) + c - '0';
                c = iter_take_pat(s, iter);
            }
        }
    }

    if (VA_BGET(s->opt, VA_OPT_STATE) == VA_STATE_PREC) {
        VA_BSET(s->opt, VA_OPT_STATE, VA_STATE_ARG);

        /* parse sizes and quotation */
        for(;;) {
            switch (c) {
            default:
                goto end_of_size;
            case 'h':
                if (VA_BGET(s->opt, VA_OPT_SIZE) >= 2) {
                    va_stream_set_error(s, VA_E_FORMAT);
                }
                else {
                    VA_BSET(s->opt, VA_OPT_SIZE, VA_BGET(s->opt, VA_OPT_SIZE) + 1);
                }
                break;
            case 'z':
                VA_BSET(s->opt, VA_OPT_SIGN, VA_SIGN_ZEXT);
                break;
            case 'k':
                /* k=1, kk=5: (old << 2) + 1 */
                VA_BSET(s->opt, VA_OPT_QUOTE, (VA_BGET(s->opt, VA_OPT_QUOTE) << 2) + VA_QUOTE_k);
                break;
            case 'q':
                /* q=2, qq=6: (old << 1) + 2 */
                VA_BSET(s->opt, VA_OPT_QUOTE, (VA_BGET(s->opt, VA_OPT_QUOTE) << 1) + VA_QUOTE_q);
                break;
            case 'Q':
                /* Q=3, QQ=7: (old << 1) | 3 */
                VA_BSET(s->opt, VA_OPT_QUOTE, (VA_BGET(s->opt, VA_OPT_QUOTE) << 1) | VA_QUOTE_Q);
                break;
            case 'K':
                VA_BSET(s->opt, VA_OPT_QUOTE, VA_QUOTE_K);
                break;
            }
            c = iter_take_pat(s, iter);
        }
end_of_size:

        /* parse format */
        switch (c) {
        case 'h': case 'H':
        case 'z': case 'Z':
        case 'q': case 'Q':
        case 'k': case 'K':
        default:
            va_stream_set_error(s, VA_E_FORMAT);
            VA_BSET(s->opt, VA_OPT_STATE, VA_STATE_SKIP);
            break;

        case VA_SIGIL:
            if (s->width == VA_WIDTH_NONE) {
                s->width = (s->opt & VA_OPT_ZERO) ? 0 : 1;
            }
            while (s->width > 0) {
                render(s, VA_SIGIL);
            }
            goto again;

        case 's': case 'S':
            break;
        case 'a': case 'A':
            break;
        case 'f': case 'F':
            break;
        case 'g': case 'G':
            break;
        case 'b': case 'B':
            VA_BSET(s->opt, VA_OPT_BASE, 2);
            break;
        case 'o': case 'O':
            VA_BSET(s->opt, VA_OPT_BASE, 8);
            break;
        case 'u': case 'U':
            VA_BSET(s->opt, VA_OPT_SIGN, VA_SIGN_ZEXT);
            VA_BSET(s->opt, VA_OPT_BASE, 10);
            break;
        case 'd': case 'D':
        case 'i': case 'I':
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
            s->opt ^= VA_OPT_VAR;
            break;
        case 't': case 'T':
            VA_BSET(s->opt, VA_OPT_MODE, VA_MODE_TYPE);
            break;
        case 'e': case 'E':
            VA_BSET(s->opt, VA_OPT_MODE, VA_MODE_B32);
            VA_BSET(s->opt, VA_OPT_BASE, 32);
            break;
        }

        if ((c >= 'a') && (c <= 'z')) {
            s->pat = *iter;
        }
        else
        if ((c >= 'A') && (c <= 'Z')) {
            s->opt |= VA_OPT_UPPER;
            s->pat = *iter;
        }
    }

    if (s->width == VA_WIDTH_NONE) {
        s->width = 0;
    }
    assert((VA_BGET(s->opt, VA_OPT_STATE) == VA_STATE_ARG) ||
           (VA_BGET(s->opt, VA_OPT_STATE) == VA_STATE_SKIP));
    /* if there is a '=' option, repeat printing */
    if (s->opt & VA_OPT_EQUAL) {
        return 1;
    }

ast_arg:
    /* if there are no more args, eat up the whole format string */
    if (s->opt & VA_OPT_LAST) {
        return 2;
    }
    /* exit printer loop, i.e., read next arg */
    return 0;

end_of_format:
    s->width = 0;
    s->prec = VA_PREC_NONE;
    s->opt &= VA_OPT_RESET_END;
    /* exit printer loop at end of string */
    return 0;
}

static bool set_ast(va_stream_t *s, long long x)
{
    switch (VA_BGET(s->opt, VA_OPT_STATE)) {
    case VA_STATE_WIDTH:
        if (x < 0) {
            s->opt |= VA_OPT_MINUS;
            x = -x;
        }
        s->width = (x >= VA_WIDTH_MAX) ? VA_WIDTH_MAX : (x & VA_WIDTH_MASK);
        return 0;

    case VA_STATE_PREC:
        s->prec = (x >= VA_PREC_MAX) ? VA_PREC_MAX : (x & VA_PREC_MASK);
        return 0;

    case VA_STATE_SKIP:
        VA_BSET(s->opt, VA_OPT_STATE, VA_STATE_ARG);
        return 0;

    case VA_STATE_INIT:
        va_stream_set_error(s, VA_E_ARGC);
        return 0;
    }

    return 1;
}

static void ensure_init(va_stream_t *s)
{
    /* init */
    if (s->vtab->init != NULL) {
        s->vtab->init(s);
    }

    /* first format */
    if (VA_BGET(s->opt, VA_OPT_STATE) == VA_STATE_INIT) {
        parse_format(s);
    }
}

#define RENDER_LOOP(s, astval, render) \
    do{ \
        ensure_init(s); \
        unsigned u; \
        do { \
            if (set_ast(s, astval)) { \
                render; \
            } \
            while ((u = parse_format(s)) >= 2) {} \
        } while (u); \
    }while(0)

static va_stream_t *xprintf_sll(va_stream_t *s, long long x, unsigned sz)
{
    RENDER_LOOP(s, x, render_sll(s, x, sz));
    return s;
}

static va_stream_t *xprintf_ull(va_stream_t *s, unsigned long long x, unsigned sz)
{
    RENDER_LOOP(s, (long long)x, render_ull(s, x, sz));
    return s;
}

static va_stream_t *xprintf_bool(va_stream_t *s, unsigned long long x)
{
    RENDER_LOOP(s, (long long)x, render_bool(s, x));
    return s;
}

/* ********************************************************************** */
/* extern functions */

extern va_stream_t *va_xprintf_iter(
    va_stream_t *s,
    va_read_iter_t *x)
{
    void const *start = x->cur;
    RENDER_LOOP(s, 0, render_iter(s, x, start));
    return s;
}

extern va_stream_t *va_xprintf_custom(
    va_stream_t *s,
    va_print_t *x)
{
    RENDER_LOOP(s, 0, render_custom(s, x));
    return s;
}

extern va_stream_t *va_xprintf_ptr(va_stream_t *s, void const *x)
{
    RENDER_LOOP(s, 0, render_ptr(s, x));
    return s;
}

extern va_stream_t *va_xprintf_ull(va_stream_t *s, unsigned long long x)
{
    return xprintf_ull(s, x, sizeof(x));
}

extern va_stream_t *va_xprintf_ulong(va_stream_t *s, unsigned long x)
{
    return xprintf_ull(s, x, sizeof(x));
}

extern va_stream_t *va_xprintf_bool(va_stream_t *s, unsigned x)
{
    return xprintf_bool(s, x);
}

extern va_stream_t *va_xprintf_uint(va_stream_t *s, unsigned x)
{
    return xprintf_ull(s, x, sizeof(x));
}

extern va_stream_t *va_xprintf_ushort(va_stream_t *s, unsigned short x)
{
    return xprintf_ull(s, x, sizeof(x));
}

extern va_stream_t *va_xprintf_uchar(va_stream_t *s, unsigned char x)
{
    return xprintf_ull(s, x, sizeof(x));
}

extern va_stream_t *va_xprintf_sll(va_stream_t *s, long long x)
{
    return xprintf_sll(s, x, sizeof(x));
}

extern va_stream_t *va_xprintf_slong(va_stream_t *s, long x)
{
    return xprintf_sll(s, x, sizeof(x));
}

extern va_stream_t *va_xprintf_sint(va_stream_t *s, int x)
{
    return xprintf_sll(s, x, sizeof(x));
}

extern va_stream_t *va_xprintf_short(va_stream_t *s, short x)
{
    return xprintf_sll(s, x, sizeof(x));
}

extern va_stream_t *va_xprintf_schar(va_stream_t *s, signed char x)
{
    return xprintf_sll(s, x, sizeof(x));
}

extern va_stream_t *va_xprintf_char(va_stream_t *s, char x)
{
    if ((char)-1 < 0) {
        return va_xprintf_schar(s, (signed char)x);
    }
    else {
        return va_xprintf_uchar(s, (unsigned char)x);
    }
}

extern unsigned va_stream_get_error_f(va_stream_t const *s)
{
    return VA_BGET(s->opt, VA_OPT_ERR);
}

extern va_stream_t *va_xprintf_error_t_p(va_stream_t *s, va_error_t *x)
{
    ensure_init(s);
    x->code = VA_BGET(s->opt, VA_OPT_ERR);
    VA_BSET(s->opt, VA_OPT_ERR, 0);
    return s;
}

extern va_stream_t *va_xprintf_last_schar(va_stream_t *s, signed char x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_schar(s,x);
}

extern va_stream_t *va_xprintf_last_short(va_stream_t *s, short x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_short(s,x);
}

extern va_stream_t *va_xprintf_last_sint(va_stream_t *s, int x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_sint(s,x);
}

extern va_stream_t *va_xprintf_last_slong(va_stream_t *s, long x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_slong(s,x);
}

extern va_stream_t *va_xprintf_last_sll(va_stream_t *s, long long x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_sll(s,x);
}

extern va_stream_t *va_xprintf_last_uchar(va_stream_t *s, unsigned char x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_uchar(s,x);
}

extern va_stream_t *va_xprintf_last_ushort(va_stream_t *s, unsigned short x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_ushort(s,x);
}

extern va_stream_t *va_xprintf_last_bool(va_stream_t *s, unsigned int x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_bool(s,x);
}

extern va_stream_t *va_xprintf_last_uint(va_stream_t *s, unsigned int x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_uint(s,x);
}

extern va_stream_t *va_xprintf_last_ulong(va_stream_t *s, unsigned long x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_ulong(s,x);
}

extern va_stream_t *va_xprintf_last_ull(va_stream_t *s, unsigned long long x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_ull(s,x);
}

extern va_stream_t *va_xprintf_last_ptr(va_stream_t *s, void const *x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_ptr(s,x);
}

extern va_stream_t *va_xprintf_last_char(va_stream_t *s, char x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_char(s,x);
}

extern va_stream_t *va_xprintf_last_iter(va_stream_t *s, va_read_iter_t *x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_iter(s,x);
}

extern va_stream_t *va_xprintf_last_custom(va_stream_t *s, va_print_t *x)
{
    s->opt |= VA_OPT_LAST;
    return va_xprintf_custom(s,x);
}

extern va_stream_t *va_xprintf_last_error_t_p(va_stream_t *s, va_error_t *x)
{
    ensure_init(s);
    if (VA_BGET(s->opt, VA_OPT_STATE) != VA_STATE_INIT) {
        /* we're not reading anything, so there are too few arguments. */
        va_stream_set_error(s, VA_E_ARGC);
    }
    s->opt |= VA_OPT_LAST;
    while (parse_format(s) >= 2) {}
    return va_xprintf_error_t_p(s,x);
}

extern va_stream_t *va_xprintf_init_last(
    va_stream_t *s,
    void const *x,
    va_read_iter_vtab_t const *get_vtab)
{
    va_xprintf_init(s, x, get_vtab);
    ensure_init(s);
    s->opt |= VA_OPT_LAST;
    while (parse_format(s) >= 2) {}
    return s;
}

extern char const *va_strerror(unsigned u)
{
    static char const *const name[] = {
#define EACH(X) [X] = #X,
VA_E_FOREACH
#undef EACH
    };
    if (u < va_countof(name)) {
        return name[u];
    }
    return NULL;
}
