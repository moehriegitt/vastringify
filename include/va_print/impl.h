/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This is some internal stuff needed when va_stream_t is used
 * in implementations.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_IMPL_H_
#define VA_PRINT_IMPL_H_

#include <va_print/base.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* macros */

/* bit magic */

#define VA_MASK2(S,M)  (M)
#define VA_MASK1(W)    (VA_MASK2 W)
#define VA_MASK(W)     (VA_MASK1(W))

#define VA_SHIFT2(S,M) (S)
#define VA_SHIFT1(W)   (VA_SHIFT2 W)
#define VA_SHIFT(W)    (VA_SHIFT1(W))

#define VA_MASH2(S,M)  ((M) << (S))
#define VA_MASH1(W)    (VA_MASH2 W)
#define VA_MASH(W)     (VA_MASH1(W))

#define VA_BGET(X,W)   (((X) >> VA_SHIFT(W)) & VA_MASK(W))
#define VA_BPUT(X,W,V) ((((X) | VA_MASH(W)) ^ VA_MASH(W)) | ((V) << VA_SHIFT(W)))

#define VA_BSET(X,W,V) \
    __extension__({ \
        __typeof__(X) *_xp = &(X); \
        *_xp = VA_BPUT(*_xp,W,V); \
    })

#define VA_MSET(X,MASK,V) \
    __extension__({ \
        __typeof__(X) *_xp = &(X); \
        __typeof__(X) _m = (MASK); \
        *_xp |= _m; \
        *_xp ^= _m; \
        *_xp |= (V); \
   })

#define VA_MSET_IF(X,MASK,COND) \
    __extension__({ \
        __typeof__(X) *_xp = &(X); \
        __typeof__(X) _m = (MASK); \
        *_xp |= _m; \
        if (!(COND)) { *_xp ^= _m; } \
   })

#define VA_MCLR(X,MASK) VA_MSET_IF(X,MASK,0)

/* option bits in va_stream_t::opt */

/** '#' modifier */
#define VA_OPT_VAR    0x0001
/** upper case format specifier */
#define VA_OPT_UPPER  0x0002
/** '0' modifier */
#define VA_OPT_ZERO   0x0004
/** '-' modifier */
#define VA_OPT_MINUS  0x0008
/** '=' modifier */
#define VA_OPT_EQUAL  0x0010
/** internal: simulate printing (for counting) */
#define VA_OPT_SIM    0x0020
/** last argument: terminate format string reading */
#define VA_OPT_LAST   0x0040
/** reserved */
#define VA_OPT_X7     0x0080

/** sign options */
#define VA_OPT_SIGN   (8, 3U)
/** no sign modifier */
#define VA_SIGN_NORM  0
/** ' ' modifier */
#define VA_SIGN_SPACE 1
/** '+' modifier */
#define VA_SIGN_PLUS  2
/** 'z' modifier */
#define VA_SIGN_ZEXT  3

/** format mode */
#define VA_OPT_MODE   (10, 7U)
#define VA_MODE_NORM  0
#define VA_MODE_B32   1
#define VA_MODE_CHAR  2
#define VA_MODE_PTR   3
#define VA_MODE_TYPE  4
#define VA_MODE_STAT  5
#define VA_MODE_X6    6
#define VA_MODE_X7    7

/* internal states for parsing format specifiers */
#define VA_OPT_STATE   (13, 7U)
#define VA_STATE_INIT  0 /* before and after full scan of format string */
#define VA_STATE_ARG   1 /* ready to print an argument */
#define VA_STATE_WIDTH 2 /* after the width or '*' */
#define VA_STATE_PREC  3 /* after the prec or '.*' */
#define VA_STATE_SKIP  4 /* next arg should be skipped */

/** quotation */
#define VA_OPT_QUOTE   (16, 7U)
#define VA_QUOTE_NONE  0
#define VA_QUOTE_k     1
#define VA_QUOTE_q     2
#define VA_QUOTE_Q     3
#define VA_QUOTE_K     4
#define VA_QUOTE_kk    5
#define VA_QUOTE_qq    6
#define VA_QUOTE_QQ    7

/** size specifier mask: normal, 'h', or 'hh'*/
#define VA_OPT_SIZE   (19, 3U)

/** form (2..36) */
#define VA_OPT_BASE   (21, 0x3fU)

/** error (0..7) */
#define VA_OPT_ERR    (27, 7U)

/** number of following error code units from the decoder: 0..2 */
#define VA_OPT_EMORE  (30, 3U)

/** mask of resetting print options at end of string */
#define VA_OPT_RESET_END VA_MASH(VA_OPT_ERR)

/** mask of resetting print options at beginning of new argument */
#define VA_OPT_RESET_ARG (VA_MASH(VA_OPT_ERR) | VA_OPT_LAST)

/** mask for casting width */
#define VA_WIDTH_MASK 0x7fffffff
/** width is not given */
#define VA_WIDTH_NONE VA_WIDTH_MASK
/** maximum supported width */
#define VA_WIDTH_MAX (VA_WIDTH_MASK-1)

/** mask for casting precisino */
#define VA_PREC_MASK 0x7fffffff
/** precision is not given */
#define VA_PREC_NONE VA_PREC_MASK
/** maximum supported precision */
#define VA_PREC_MAX  (VA_PREC_MASK-1)

/* custom quotation */

/**
 * Make a 'delim' entry for a quotation.
 *
 * This contains the width of the total quotation
 */
#define VA_DELIM(prefix, delimfront, delimback) \
    (((0ULL|(prefix)) << 8) | \
     ((0ULL|(delimfront)) << 16) | \
     ((0ULL|(delimback)) << 32))

#define VA_DELIM_WIDTH(x)  ((unsigned char)  ((x) >> 0))
#define VA_DELIM_PREFIX(x) ((unsigned char)  ((x) >> 8))
#define VA_DELIM_FRONT(x)  ((unsigned short) ((x) >> 16))
#define VA_DELIM_BACK(x)   ((unsigned short) ((x) >> 32))

/**
 * A virtual table for custom quotation.
 *
 * Note that a quotation method needs va_print/impl.h to get
 * access to va_stream_render*(), va_stream_put(),
 * va_stream_set_error(), va_u_valid(), and for checking
 * the current stream options to check (s->opt & VA_OPT_ZERO) etc.
 */
typedef struct {
    /**
     * The quotation delimiter for char (in [0]) and string (in [1])
     *
     * See 'VA_DELIM(prefix, frontquote, backquote) macro for
     * contructing this.  prefix is in the range 0..0xff, frontquote
     * and backquote may be 0..0xffff each.
     *
     * If 'prefix' in VA_DELIM() is set to 0xff, then if the `z`
     * modifier is in use, then the size prefix of the string
     * element is used instead .  I.e., "", "u", or "U" depending
     * on whether it's char, char16_t, or char32_t.  Yes, this means
     * that you cannot use LATIN SMALL LETTER Y WITH DIAERESIS as a
     * string prefix character.
     */
    unsigned long long delim[2];

    /**
     * Whether a character causes the string to need quotation.
     *
     * If this is NULL, the string is always quoted (unless the '#'
     * modifier is given).
     *
     * Note that the passed character is in internal encoding:
     * the lower 24 bits are the actual codepoint payload, while
     * the upper bits are VA_U_* bits.
     *
     * Like in render_quote, context may be stored in s->qctxt.
     *
     * Checking is terminated using the check_flush method.  This
     * together with s->qctxt can be used to quote empty strings
     * (as needed for shell quotation).
     */
    bool (*check_quote)(va_stream_t *s, unsigned);

    /**
     * Terminate checking for quotation.
     *
     * This may be NULL if not needed.
     *
     * This is not invoked if the string is NULL, only if the string
     * is empty ("").  I.e., NULL will not be quoted.
     *
     * Note that this will not be invoked if check_quote() already
     * returned true.
     */
    bool (*check_flush)(va_stream_t *s);

    /**
     * Render/quote a single character.
     *
     * During printing, the quotation mechanism may use s->qctxt
     * for storing state and context information, if necessary.
     * Rendering always starts with s->qctxt reset to 0.  After
     * all characters are rendered, render_flush() is invoked
     * to terminate the rendering.  Note that rendering may
     * be stopped for internal reasons before the end of the
     * string, e.g., when counting the width of the string.
     * In these situations, render_flush() may be invoked
     * prematurely.  For the actualy rendering, however, the
     * whole sequence will be invoked.
     *
     * The passed character is in internal encoding:
     * the lower 24-bit are the actual codepoint payload, while
     * the upper bits are VA_U_* bits.
     *
     * For putting characters into the output stream, this must
     * invoke va_stream_put() or va_stream_render_*() functions.
     *
     * This function can determine whether and how the character
     * should be quoted.  It should also determine based on the
     * VA_U_* bits whether U+FFFD should be used instead for
     * printing.
     */
    void (*render_quote)(va_stream_t *, unsigned);

    /**
     * After a string is printed, notify the printer of the
     * end of the string (maybe there is more to print to
     * quote correctly).
     */
    void (*render_flush)(va_stream_t *);
} va_quotation_t;

/* ********************************************************************** */
/* debug and analysis stuff (internal) */

#ifdef VA_STACK
#define VA_POSSIBLE_CALL(x) __asm__ volatile("// possible_call:" #x)
#else
#define VA_POSSIBLE_CALL(x)
#endif

/* ********************************************************************** */
/* extern functions */

/**
 * Put a single single character into the output stream.
 *
 * This may also count only, depending on stream mode.
 *
 * This or any other va_stream_render*() functino must be used by quotation
 * methods for putting characters into the output stream -- quotation
 * methods must not try to be smart and implement their own function for
 * putting characters into the output stream.
 */
extern void va_stream_render(va_stream_t *s, unsigned c);

/**
 * Uses va_stream_put() to render a three-digit octal escape
 * sequence like in C, e.g.: \012
 */
extern void va_stream_render_quote_oct(
    va_stream_t *s,
    unsigned c);

/**
 * Uses va_stream_put() to render a \u plus four hexadecimal digits
 * or \U plus eight hexadecimal digits, e.g.: \u2000 or \U0010FFF0.
 */
extern void va_stream_render_quote_u(
    va_stream_t *s,
    unsigned c);

/**
 * Set or reset the quotation method VA_QUOTE_{k,q,Q,K,kk,qq,QQ}.
 * You can even set quotation method 0 to switch normal ~a into
 * quotation mode.
 *
 * This returns the previously set value.
 *
 * If quotation == NULL, the corresponding entry will be reset to 'no quotation'.
 */

extern va_quotation_t const *va_quotation_set(
    unsigned which,
    va_quotation_t const *quotation);

/* ********************************************************************** */
/* static inline functions */

__attribute__((always_inline))
static inline bool va_u_valid(unsigned c)
{
    return ((c < VA_U_SURR_MIN) || (c > VA_U_SURR_MAX)) && (c <= VA_U_MAX);
}

__attribute__((always_inline))
static inline void va_stream_set_error(va_stream_t *s, unsigned e)
{
    if (VA_BGET(s->opt, VA_OPT_ERR) == 0) {
        VA_BSET(s->opt, VA_OPT_ERR, e);
    }
}

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_IMPL_H_ */
