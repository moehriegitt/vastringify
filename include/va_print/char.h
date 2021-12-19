/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This prints into char[] arrays.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_CHAR_H_
#define VA_PRINT_CHAR_H_

#include <va_print/core.h>

/* ********************************************************************** */
/* defaults */

#ifndef va_char_p_encode
#include <va_print/char_utf8.h>
/** String encoding for printing into 'char' arrays. */
#define va_char_p_encode utf8
#endif

#ifndef va_char16_p_encode
#include <va_print/char_utf16.h>
/** String encoding for printing into 'char16_t' arrays. */
#define va_char16_p_encode utf16
#endif

#ifndef va_char32_p_encode
#include <va_print/char_utf32.h>
/** String encoding for printing into 'char32_t' arrays. */
#define va_char32_p_encode utf32
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* macros */

/**
 * Select a char put function based on a string type */
#define va_char_p_vtab_gen(x) _Generic(x, \
    char *:&VA_CONCAT(va_char_p_vtab_,va_char_p_encode), \
    char16_t *:&VA_CONCAT(va_char16_p_vtab_,va_char16_p_encode), \
    char32_t *:&VA_CONCAT(va_char32_p_vtab_,va_char32_p_encode))

/**
 * Value of type va_string_char_p_t */
#define VA_STREAM_CHAR_P(S,N) \
    ((va_stream_char_p_t){ \
        VA_STREAM(va_char_p_vtab_gen(S)), (S), (N), 0 })

/**
 * Print into given char array up to a given length.
 *
 * The resulting string is always NUL-terminated, but may
 * be truncated if too much is printed.
 *
 * The required size can be determined with va_gcprintf(),
 * va_cprintf(), va_ucprintf(), or va_Ucprintf().
 *
 * The length \p N given to this function must not be 0,
 * otherwise the resulting string will not be NUL terminated.
 *
 * If the pointer \p S is NULL, this prints nothing.
 *
 * This returns S filled with the result.
 */
#define va_snprintf(S,N,...) \
    VA_BLOCK_EXPR((__typeof__(*(S))*)(va_xprintf( \
        &VA_STREAM_CHAR_P(S,N), __VA_ARGS__)->data))

/**
 * Print into given char array.
 *
 * The size is determined by using va_countof() on the given
 * array parameter, otherwise this is like va_snprintf().
 *
 * The required size can be determined with va_gcprintf(),
 * va_cprintf(), va_ucprintf(), or va_Ucprintf().
 *
 * This returns S filled with the result.
 */
#define va_sprintf(S,...) va_snprintf(S,va_countof(S),__VA_ARGS__)

/**
 * Print into new char array up to a given length.
 *
 * This generates a new compound literal char[N] and prints
 * into it like va_snprintf().
 *
 * The required size can be determined with va_cprintf().
 *
 * This returns a char* pointer with the result.
 */
#define va_nprintf(N,...) \
    ((char*)(va_xprintf( \
        &VA_STREAM_CHAR_P((char[N]){0}, N), __VA_ARGS__)->data))

/**
 * Print into new char16_t array up to a given length.
 *
 * This generates a new compound literal char16_t[N] and prints
 * into it like va_snprintf().
 *
 * The required size can be determined with va_ucprintf().
 *
 * This returns a char* pointer with the result.
 */
#define va_unprintf(N,...) \
    ((char16_t*)(va_xprintf( \
        &VA_STREAM_CHAR_P((char16_t[N]){0}, N), __VA_ARGS__)->data))

/**
 * Print into new char32_t array up to a given length.
 *
 * This generates a new compound literal char32_t[N] and prints
 * into it like va_snprintf().
 *
 * The required size can be determined with va_Ucprintf().
 *
 * This returns a char* pointer with the result.
 */
#define va_Unprintf(N,...) \
    ((char32_t*)(va_xprintf( \
        &VA_STREAM_CHAR_P((char32_t[N]){0}, N), __VA_ARGS__)->data))

/**
 * Count the number of 'Char' needed to represent the output
 * string (mnemonic: siZe).
 *
 * This simulates printing like va_snprintf() and returns the number of
 * 'Char' typed elements that are needed to print the whole result.  The
 * resulting size can be used to allocate an array that fits
 * the resulting encoded string tightly.
 *
 * Note: this returns the length of the string in bytes plus 1,
 * i.e., the needed array size including the NUL termination.
 */
#define va_zprintf(Char, ...) \
    VA_BLOCK_EXPR(va_xprintf( \
        &VA_STREAM_CHAR_P((Char*)NULL, -(size_t)2), __VA_ARGS__)->pos + 1)

/* ********************************************************************** */
/* types */

typedef struct {
    va_stream_t s;
    void *data;
    size_t size;
    size_t pos;
} va_stream_char_p_t;

/* ********************************************************************** */
/* extern functions */

extern void const *va_char_p_end(
    va_read_iter_t *,
    size_t);

extern void va_char_p_init(
    va_stream_t *s);

extern void va_char_p_put(
    va_stream_t *,
    char);

extern void const *va_char16_p_end(
    va_read_iter_t *,
    size_t);

extern void va_char16_p_init(
    va_stream_t *s);

extern void va_char16_p_put(
    va_stream_t *,
    char16_t);

extern void const *va_char32_p_end(
    va_read_iter_t *,
    size_t);

extern void va_char32_p_init(
    va_stream_t *s);

extern void va_char32_p_put(
    va_stream_t *,
    char32_t);

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_CHAR_H_ */
