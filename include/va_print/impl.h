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

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* macros */

/* bit magic */

#define VA_MASK_(S,M)  (M)
#define VA_MASK(W)     (VA_MASK_ W)

#define VA_SHIFT_(S,M) (S)
#define VA_SHIFT(W)    (VA_SHIFT_ W)

#define VA_MASH_(S,M)  ((M) << (S))
#define VA_MASH(W)     (VA_MASH_ W)

#define VA_BGET(X,W)   (((X) >> VA_SHIFT(W)) & VA_MASK(W))
#define VA_BPUT(X,W,V) ((((X) | VA_MASH(W)) ^ VA_MASH(W)) | ((V) << VA_SHIFT(W)))

#define VA_BSET(X,W,V) \
    ({ \
        __typeof__(X) *_xp = &(X); \
        *_xp = VA_BPUT(*_xp,W,V); \
    })

#define VA_MSET(X,MASK,V) \
    ({ \
        __typeof__(X) *_xp = &(X); \
        __typeof__(X) _m = (MASK); \
        *_xp |= _m; \
        *_xp ^= _m; \
        *_xp |= (V); \
   })

#define VA_MSET_IF(X,MASK,COND) \
    ({ \
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
/** '+' modifier */
#define VA_OPT_PLUS   0x0010
/** ' ' modifier */
#define VA_OPT_SPACE  0x0020
/** precision was given ('.' modifier) */
#define VA_OPT_PREC   0x0040
/** '=' modifier */
#define VA_OPT_EQUAL  0x0080

/** mode */
#define VA_OPT_MODE   (9, 3U)
#define VA_MODE_NORM  0
#define VA_MODE_B32   1
#define VA_MODE_CHAR  2
#define VA_MODE_PTR   3

/** quotation */
#define VA_OPT_QUOTE    0x1800
/** 'Q' modifier */
#define VA_OPT_QUOTE_SH 0x0800
/** 'q' modifier */
#define VA_OPT_QUOTE_C  0x1000
/** 'qq' modifier */
#define VA_OPT_QUOTE_J  0x1800
/** bit for C or J quotation */
#define VA_OPT_QUOTE_CJ 0x1000

/* internal states for parsing format specifiers */
#define VA_OPT_STATE   0xe000
#define VA_OPT_STATE0  0x0000 /* in normal string, '%' starts seq */
#define VA_OPT_STATE1  0x2000 /* after the width, '.' start prec */
#define VA_OPT_STATE2  0x4000
#define VA_OPT_STATE3  0x6000 /* after the prec, form char follows */
#define VA_OPT_STATE4  0x8000
#define VA_OPT_STATE5  0xa000 /* '*' for width was parsed */
#define VA_OPT_STATE6  0xc000
#define VA_OPT_STATE7  0xe000 /* '*' for prec was parsed */

/** size specifier mask */
#define VA_OPT_SIZE   0x30000
/** 'h' size specifier */
#define VA_OPT_SIZE1  0x10000
/** 'hh' size specifier */
#define VA_OPT_SIZE2  0x20000

/** form (2..36) */
#define VA_OPT_BASE   (18, 0x3fU)

/** error (0..7) */
#define VA_OPT_ERR    (24, 7U)

/** mask of resetting print options, without resetting an error */
#define VA_OPT_RESET  VA_EXP1(VA_MASH(VA_OPT_ERR))

/** number of following error code units from the decoder */
#define VA_OPT_EMORE  (27, 7U)

/* ********************************************************************** */
/* static inline functions */

static inline bool va_u_valid(unsigned c)
{
    return ((c < VA_U_SURR_MIN) || (c > VA_U_SURR_MAX)) && (c <= VA_U_MAX);
}

static inline void va_read_iter_start(va_read_iter_t *iter)
{
    iter->cur = iter->start;
}

static inline unsigned va_read_iter_take(va_read_iter_t *iter)
{
    return iter->take(iter);
}

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
