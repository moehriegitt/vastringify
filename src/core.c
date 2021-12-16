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

static void render(va_stream_t *s, unsigned c)
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

static void render_quote_oct(
    va_stream_t *s,
    unsigned c)
{
    render(s, '\\');
    render(s, '0' + ((c >> 6) & 7));
    render(s, '0' + ((c >> 3) & 7));
    render(s, '0' + ((c >> 0) & 7));
}

static void render_quote_unicode(
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
    iter->cur = start;
}

static unsigned iter_take(
    va_stream_t *s,
    va_read_iter_t *iter,
    void const *end)
{
    if (iter->cur == NULL) {
        va_stream_set_error(s, VA_E_NULL);
        return 0;
    }

    unsigned c = iter->vtab->take(iter, end);

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
    unsigned c)
{
    unsigned c2;
    switch (VA_BGET(s->opt, VA_OPT_QUOTE)) {
    case VA_QUOTE_C:
    case VA_QUOTE_J:
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
            if (VA_BGET(s->opt,VA_OPT_QUOTE) == VA_QUOTE_C) {
                render_quote_oct(s, c);
            }
            else {
                render_quote_unicode(s, c);
            }
            return;
        }
        if (((s->opt & VA_OPT_ZERO) != 0) && (c >= 0x80)) {
            render_quote_unicode(s, c);
            return;
        }
        break;

    case VA_QUOTE_SH:
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
        case VA_QUOTE_C: render_rawstr(s, "NULL"); return;
        case VA_QUOTE_J: render_rawstr(s, "null"); return;
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
    unsigned delim = 0; /* byte 0 is count, byte 1 is prefix, byte 2 is delim */
    if ((s->opt & VA_OPT_VAR) == 0) {
        switch (VA_BGET(s->opt, VA_OPT_QUOTE)) {
        case VA_QUOTE_C:
            delim = (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR) ? '\'' : '"';
            delim = 2 + (delim << 16);
            if ((VA_BGET(s->opt, VA_OPT_SIGN) == VA_SIGN_ZEXT) &&
                (iter->vtab->str_prefix))
            {
                delim += 1 + ((unsigned)(iter->vtab->str_prefix) << 8);
            }
            break;
        case VA_QUOTE_J:
            delim = (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR) ? '\'' : '"';
            delim = 2 + (delim << 16);
            break;
        case VA_QUOTE_SH:
            for (iter_start(s,iter,start); (ch = iter_take(s,iter,end)) != 0;) {
                if (va_quote_sh(ch)) {
                    delim = '\'';
                    delim = 2 + (delim << 16);
                    break;
                }
            }
            break;
        }
    }

    /* reinterpret 'width' into how many spaces are written */
    if ((s->opt & VA_OPT_MINUS) == 0) {
        if (s->width <= (delim & 0xff)) {
            s->width = 0;
        }
        else {
            s->width -= (delim & 0xff);
            s->opt |= VA_OPT_SIM;
            iter_start(s,iter,start);
            while ((s->width > 0) && ((ch = iter_take(s,iter,end)) != 0)) {
                render_quotec(s, ch);
            }
            VA_MCLR(s->opt, VA_OPT_SIM);

            /* space */
            while (s->width > 0) {
                render(s, ' ');
            }
        }
    }

    /* meat */
    render(s, (delim >> 8)  & 0xff);
    render(s, (delim >> 16) & 0xff);
    for (iter_start(s,iter,start); (ch = iter_take(s,iter,end)) != 0;) {
        render_quotec(s, ch);
    }
    render(s, (delim >> 16) & 0xff);

    /* space */
    while (s->width > 0) {
        render(s, ' ');
    }
}

static void render_iter(va_stream_t *s, va_read_iter_t *iter)
{
    switch (VA_BGET(s->opt, VA_OPT_MODE)) {
    case VA_MODE_TYPE:
        render_rawstr(s, iter->vtab->type);
        return;
    case VA_MODE_PTR:
        render_ptr(s, iter->cur);
        return;
    }
    render_iter_algo(s, iter);
    return;
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
        return 0;
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
    0,
    {0},
};

static va_read_iter_vtab_t const arr1_vtab_16 = {
    NULL,
    arr1_utf64_take,
    arr1_utf64_end,
    'u',
    {0},
};

static va_read_iter_vtab_t const arr1_vtab_32 = {
    NULL,
    arr1_utf64_take,
    arr1_utf64_end,
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
    case 3: x &= 0xf; break;
    }

    if (VA_BGET(s->opt, VA_OPT_MODE) == VA_MODE_CHAR) {
        render_char(s, x, sz);
        return;
    }
    render_int(s, x, 10, 0);
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
static bool parse_format(va_stream_t *s)
{
    va_read_iter_t iter[1] = { s->pat };

again:;
    unsigned c = iter_take(s, iter, NULL);
    /* stay at end of string, regardless of STATE */
    if (c == 0) {
        s->width = 0;
        s->prec = VA_PREC_NONE;
        s->opt &= VA_OPT_RESET;
        return 0;
    }

    if (VA_BGET(s->opt, VA_OPT_STATE) == 0) {
        s->width = 0;
        s->prec = VA_PREC_NONE;
        s->opt &= VA_OPT_RESET;

        /* print part between format specifiers */
        for (;;) {
            if (c == 0) {
                return 0;
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

    if (VA_BGET(s->opt, VA_OPT_STATE) == VA_STATE_WIDTH) {
        VA_BSET(s->opt, VA_OPT_STATE, VA_STATE_PREC);

        /* parse precision */
        if (c == '.') {
            s->prec = 0;
            c = iter_take_pat(s, iter);
            if (c == '*') {
                s->pat = *iter;
                return 0;
            }
            while ((c >= '0') && (c <= '9')) {
                s->prec = (s->prec * 10) + c - '0';
                c = iter_take_pat(s, iter);
            }
        }
    }

    if (VA_BGET(s->opt, VA_OPT_STATE) == VA_STATE_PREC) {
        VA_BSET(s->opt, VA_OPT_STATE, 0);

        /* parse sizes and quotation */
        for(;;) {
            switch (c) {
            default:
                goto end_of_size;
            case 'h':
                VA_BSET(s->opt, VA_OPT_SIZE, VA_BGET(s->opt, VA_OPT_SIZE) + 1);
                break;
            case 'z':
                VA_BSET(s->opt, VA_OPT_SIGN, VA_SIGN_ZEXT);
                break;
            case 'Q':
                VA_BSET(s->opt, VA_OPT_QUOTE, VA_QUOTE_J);
                break;
            case 'q':
                VA_BSET(s->opt, VA_OPT_QUOTE, VA_QUOTE_C);
                break;
            case 'k':
                VA_BSET(s->opt, VA_OPT_QUOTE, VA_QUOTE_SH);
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
            render(s, VA_U_OBJECT);
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
            s->opt |= VA_OPT_VAR;
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
    assert((VA_BGET(s->opt, VA_OPT_STATE) == 0) ||
           (VA_BGET(s->opt, VA_OPT_STATE) == VA_STATE_SKIP));
    return s->opt & VA_OPT_EQUAL ? 1 : 0;
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
        VA_BSET(s->opt, VA_OPT_STATE, 0);
        return 0;
    }

    return 1;
}

static va_stream_t *xprintf_sll(va_stream_t *s, long long x, unsigned sz)
{
    do {
        if (set_ast(s, x)) {
            render_sll(s, x, sz);
        }
    } while (parse_format(s));
    return s;
}

static va_stream_t *xprintf_ull(va_stream_t *s, unsigned long long x, unsigned sz)
{
    do {
        if (set_ast(s, (long long)x)) {
            render_ull(s, x, sz);
        }
    } while (parse_format(s));
    return s;
}

/* ********************************************************************** */
/* extern functions */

extern va_stream_t *va_xprintf_iter(
    va_stream_t *s,
    va_read_iter_t *x)
{
    void const *start = x->cur;
    do {
        x->cur = start;
        if (set_ast(s, 0)) {
            render_iter(s, x);
        }
    } while (parse_format(s));
    return s;
}

extern va_stream_t *va_xprintf_ptr(va_stream_t *s, void const *x)
{
    do {
        if (set_ast(s, 0)) {
            render_ptr(s, x);
        }
    } while (parse_format(s));
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

extern va_stream_t *va_xprintf_error_t_p(va_stream_t *s, va_error_t *x)
{
    x->code = VA_BGET(s->opt, VA_OPT_ERR);
    VA_BSET(s->opt, VA_OPT_ERR, 0);
    return s;
}

extern va_stream_t *va_xprintf_init(
    va_stream_t *s,
    void const *x,
    va_read_iter_vtab_t const *get_vtab)
{
    /* init */
    s->pat = VA_READ_ITER(get_vtab, x);
    if (s->vtab->init != NULL) {
        s->vtab->init(s);
    }

    /* first format */
    parse_format(s);
    return s;
}
