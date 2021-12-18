/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * Generic definitions for type-safe formatted printing in C.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_CORE_H_
#define VA_PRINT_CORE_H_

#include <va_print/base.h>

/* ********************************************************************** */
/* defaults */

#ifndef va_char_p_format
#include <va_print/utf8.h>
/** String encoding for 'char*' format strings. */
#define va_char_p_format utf8
#endif

#ifndef va_char_p_decode
#include <va_print/utf8.h>
/** String encoding for 'char*' print parameters. */
#define va_char_p_decode utf8
#endif

#ifndef va_char16_p_format
#include <va_print/utf16.h>
/** String encoding for 'char16_t*' format strings. */
#define va_char16_p_format utf16
#endif

#ifndef va_char16_p_decode
#include <va_print/utf16.h>
/** String encoding for 'char16_t*' print parameters. */
#define va_char16_p_decode utf16
#endif

#ifndef va_char32_p_format
#include <va_print/utf32.h>
/** String encoding for 'char32_t*' format strings. */
#define va_char32_p_format utf32
#endif

#ifndef va_char32_p_decode
#include <va_print/utf32.h>
/** String encoding for 'char32_t*' print parameters. */
#define va_char32_p_decode utf32
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* macros */

/**
 * Type generic printer function
 */
#define va_xprintf_gen(fun,s,x) _Generic(x, \
    char const *:VA_CONCAT(fun##char_p_,va_char_p_decode), \
    char *:VA_CONCAT(fun##char_p_,va_char_p_decode), \
    char const **:VA_CONCAT(fun##char_const_pp_,va_char_p_decode), \
    char **:VA_CONCAT(fun##char_pp_,va_char_p_decode), \
    char16_t const *:VA_CONCAT(fun##char16_p_,va_char16_p_decode), \
    char16_t *:VA_CONCAT(fun##char16_p_,va_char16_p_decode), \
    char16_t const **:VA_CONCAT(fun##char16_const_pp_,va_char16_p_decode), \
    char16_t **:VA_CONCAT(fun##char16_pp_,va_char16_p_decode), \
    char32_t const *:VA_CONCAT(fun##char32_p_,va_char32_p_decode), \
    char32_t *:VA_CONCAT(fun##char32_p_,va_char32_p_decode), \
    char32_t const **:VA_CONCAT(fun##char32_const_pp_,va_char32_p_decode), \
    char32_t **:VA_CONCAT(fun##char32_pp_,va_char32_p_decode), \
    unsigned char:fun##uchar, \
    unsigned short:fun##ushort, \
    unsigned int:fun##uint, \
    unsigned long:fun##ulong, \
    unsigned long long:fun##ull, \
    char:fun##char, \
    signed char:fun##schar, \
    short:fun##short, \
    int:fun##sint, \
    long:fun##slong, \
    long long:fun##sll, \
    va_read_iter_t *:fun##iter, \
    va_error_t *:fun##error_t_p, \
    default:fun##ptr)(s,x)

/**
 * The generic 'take' function for a given type format string
 */
#define va_format_gen(x) _Generic(x, \
    char const *:&VA_CONCAT(va_char_p_read_vtab_,va_char_p_format), \
    char *:&VA_CONCAT(va_char_p_read_vtab_,va_char_p_format), \
    char16_t const *:&VA_CONCAT(va_char16_p_read_vtab_,va_char16_p_format), \
    char16_t *:&VA_CONCAT(va_char16_p_read_vtab_,va_char16_p_format), \
    char32_t const *:&VA_CONCAT(va_char32_p_read_vtab_,va_char32_p_format), \
    char32_t *:&VA_CONCAT(va_char32_p_read_vtab_,va_char32_p_format))

/**
 * Default initialiser function for a stream that assumes
 * that the stream object that is passed can be cast to va_stream_t.
 *
 * This also invokes the 'init' function of the stream if that function
 * pointer is non-NULL.
 */
#define VA_INIT(Z,X,G) \
    va_xprintf_init((va_stream_t*)(1?(Z):((void*)0)), X, G)

/**
 * Iterative printing into the same stream.
 *
 * The first parameter is a stream that can be generated using
 * &VA_STREAM*() compound literal macros.  Any stream works,
 * e.g., VA_STREAM_CHARP() for iterative printing into a static
 * char array, or VA_STREAM_VEC() for printing into a
 * dynamically generated string.
 *
 * The stream generated with VA_STREAM*() can be stored in a
 * variable and then used to call this macro multiple times
 * to append into that same stream.
 *
 * This function returns nothing.
 *
 * The same can be done with va_xprintf(), but this macro
 * optimises for the iterative invocation, while va_xprintf()
 * is meant for temporary compound literal stream objects used
 * in only one expression.
 *
 * This is very similar to va_xprintf(), but returns nothing and
 * is wrapped in a block so that it potentially lets the compiler
 * optimise the stack usage more.
 */
#define va_iprintf(P,X,...) \
    VA_BLOCK_STMT((void)va_xprintf(P,X,__VA_ARGS__))

/**
 * Print a formatted string char by char using a printer function.
 *
 * The printer function passed as the first argument has type
 * 'va_stream_vtab_t const *' with at least the 'put' function
 * set to a function of type 'void (*)(va_stream_t *, unsigned)'.
 * The put function then receives a code unit stream of what
 * is printed.  Usually (if the corresponding input decoders
 * are used), this is based on Unicode, i.e., the 'unsigned' is a
 * Unicode code point.
 *
 * This returns nothing.
 */
#define va_pprintf(P,X,...) \
    va_iprintf(&VA_STREAM(P), X, __VA_ARGS__)

/**
 * Body of a printer function, for defining new va_...printf() macros.
 *
 * This takes a pointer to an initial stream value, a format string,
 * and arguments.  The type of the initial value is assumed to have as
 * a first slot a va_stream_t object so that pointers can be cast.
 * The result of the expression is the pointer to the initial value.
 *
 * This macro is a low-level function that can be used also if 'Zero'
 * is a compound literal that is (partially) returned to the user, as
 * there is no internal block structure.  Users that want no part of
 * the compound literals to be returned should use VA_BLOCK_STMT() or
 * VA_BLOCK_EXPR() to limit the lifetime of the compound literals to
 * this expression, so that the compiler can reuse the stack.
 */
#define va_xprintf(Zero,X,...) \
    ((__typeof__(Zero))(VA_REU( \
        va_xformat, \
        va_xinit,((Zero),X,va_format_gen(X)), __VA_ARGS__)))

#define va_xformat(i,s,a)  va_xformat ## i (s,a)
#define va_xformat0(s,a)   va_xprintf_gen(va_xprintf_last_, s, a)
#define va_xformat1(s,a)   va_xprintf_gen(va_xprintf_, s, a)

#define va_xinit(i,arg)  (va_xinit ## i arg)
#define va_xinit0(z,x,g) va_xprintf_init_last((va_stream_t*)(1?(z):((void*)0)), x, g)
#define va_xinit1(z,x,g) va_xprintf_init((va_stream_t*)(1?(z):((void*)0)), x, g)

/* ********************************************************************** */
/* extern objects */

extern va_read_iter_vtab_t const va_char_p_read_vtab_iso8859_1;

/* ********************************************************************** */
/* extern functions */

extern va_stream_t *va_xprintf_schar(va_stream_t *, signed char x);
extern va_stream_t *va_xprintf_short(va_stream_t *, short x);
extern va_stream_t *va_xprintf_sint(va_stream_t *, int x);
extern va_stream_t *va_xprintf_slong(va_stream_t *, long x);
extern va_stream_t *va_xprintf_sll(va_stream_t *, long long x);

extern va_stream_t *va_xprintf_uchar(va_stream_t *, unsigned char x);
extern va_stream_t *va_xprintf_ushort(va_stream_t *, unsigned short x);
extern va_stream_t *va_xprintf_uint(va_stream_t *, unsigned int x);
extern va_stream_t *va_xprintf_ulong(va_stream_t *, unsigned long x);
extern va_stream_t *va_xprintf_ull(va_stream_t *, unsigned long long x);

extern va_stream_t *va_xprintf_ptr(va_stream_t *, void const *);
extern va_stream_t *va_xprintf_char(va_stream_t *, char x);
extern va_stream_t *va_xprintf_error_t_p(va_stream_t *, va_error_t *x);

extern va_stream_t *va_xprintf_iter(va_stream_t *, va_read_iter_t *);

extern va_stream_t *va_xprintf_last_schar(va_stream_t *, signed char x);
extern va_stream_t *va_xprintf_last_short(va_stream_t *, short x);
extern va_stream_t *va_xprintf_last_sint(va_stream_t *, int x);
extern va_stream_t *va_xprintf_last_slong(va_stream_t *, long x);
extern va_stream_t *va_xprintf_last_sll(va_stream_t *, long long x);

extern va_stream_t *va_xprintf_last_uchar(va_stream_t *, unsigned char x);
extern va_stream_t *va_xprintf_last_ushort(va_stream_t *, unsigned short x);
extern va_stream_t *va_xprintf_last_uint(va_stream_t *, unsigned int x);
extern va_stream_t *va_xprintf_last_ulong(va_stream_t *, unsigned long x);
extern va_stream_t *va_xprintf_last_ull(va_stream_t *, unsigned long long x);

extern va_stream_t *va_xprintf_last_ptr(va_stream_t *, void const *);
extern va_stream_t *va_xprintf_last_char(va_stream_t *, char x);
extern va_stream_t *va_xprintf_last_error_t_p(va_stream_t *, va_error_t *x);

extern va_stream_t *va_xprintf_last_iter(va_stream_t *, va_read_iter_t *);

/**
 * Set the iterator. */
static inline va_stream_t *va_xprintf_init(
    va_stream_t *s,
    void const *x,
    va_read_iter_vtab_t const *get_vtab)
{
    s->pat = VA_READ_ITER(get_vtab, x);
    return s;
}

/**
 * Initialise the stream with the format string.
 *
 * This also invokes the 'init' function of the stream if that function
 * pointer is non-NULL.
 *
 * Also, this parses the format string prefix and puts it to the output
 * stream, and then parses the first % format, if present.
 */
extern va_stream_t *va_xprintf_init_last(
    va_stream_t *,
    void const *x,
    va_read_iter_vtab_t const *get_vtab);

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_CORE_H_ */
