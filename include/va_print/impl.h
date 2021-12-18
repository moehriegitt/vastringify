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
/** internal: simulate printing */
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
#define VA_MODE_X5    5
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
#define VA_OPT_QUOTE   (16, 3U)
/** no quotation */
#define VA_QUOTE_NONE  0
/** 'Q' modifier */
#define VA_QUOTE_SH    1
/** 'q' modifier */
#define VA_QUOTE_C     2
/** 'qq' modifier */
#define VA_QUOTE_J     3

/** size specifier mask: normal, 'h', or 'hh'*/
#define VA_OPT_SIZE   (18, 3U)

/** form (2..36) */
#define VA_OPT_BASE   (20, 0x3fU)

/** error (0..7) */
#define VA_OPT_ERR    (26, 7U)

/** number of following error code units from the decoder: 0..3 */
#define VA_OPT_EMORE  (29, 3U)

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
