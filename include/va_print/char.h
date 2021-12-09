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
#define VA_STREAM_CHARP(S,N) \
    ((va_stream_char_p_t){ \
        VA_STREAM(va_char_p_vtab_gen(S)), (S), (N), 0 })

/**
 * Print into given char array up to a given length.
 *
 * The resulting string is always NUL-terminated, but may
 * be truncated if too much is printed.
 *
 * The required size can be determined with va_lprintf().
 *
 * This returns S filled with the result.
 */
#define va_snprintf(S,N,...) \
    VA_BLOCK_EXPR((__typeof__(*(S))*)(va_xprintf( \
        &VA_STREAM_CHARP(S,N), __VA_ARGS__)->data))

/**
 * Print into given char array.
 *
 * The size is determined by using va_countof() on the given
 * array parameter, otherwise this is like va_snprintf().
 *
 * The required size can be determined with va_lprintf().
 *
 * This returns S filled with the result.
 */
#define va_szprintf(S,...) va_snprintf(S,va_countof(S),__VA_ARGS__)

/**
 * Print into new char array up to a given length.
 *
 * This generates a new compound literal char[N] and prints
 * into it like va_snprintf().
 *
 * The required size can be determined with va_lprintf().
 *
 * This returns a char* pointer with the result.
 */
#define va_nprintf(N,...) \
    ((char*)(va_xprintf( \
        &VA_STREAM_CHARP((char[N]){0}, N), __VA_ARGS__)->data))

/**
 * Print into new char16_t array up to a given length.
 *
 * This generates a new compound literal char16_t[N] and prints
 * into it like va_snprintf().
 *
 * The required size can be determined with va_lprintf().
 *
 * This returns a char* pointer with the result.
 */
#define va_unprintf(N,...) \
    ((char16_t*)(va_xprintf( \
        &VA_STREAM_CHARP((char16_t[N]){0}, N), __VA_ARGS__)->data))

/**
 * Print into new char32_t array up to a given length.
 *
 * This generates a new compound literal char32_t[N] and prints
 * into it like va_snprintf().
 *
 * The required size can be determined with va_lprintf().
 *
 * This returns a char* pointer with the result.
 */
#define va_Unprintf(N,...) \
    ((char32_t*)(va_xprintf( \
        &VA_STREAM_CHARP((char32_t[N]){0}, N), __VA_ARGS__)->data))

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

extern void va_char_p_init(
    va_stream_t *s);

extern void va_char_p_put(
    va_stream_t *,
    char);

extern void va_char16_p_init(
    va_stream_t *s);

extern void va_char16_p_put(
    va_stream_t *,
    char16_t);

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
