/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This prints into dynamically allocated 'char' based vectors.arrays
 * that are reallocated as printing proceeds to accomodate the whole
 * string.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_MALLOC_H_
#define VA_PRINT_MALLOC_H_

#include <va_print/core.h>

/* ********************************************************************** */
/* defaults */

#ifndef va_vec_encode
#include <va_print/malloc_utf8.h>
/** String encoding for 'char' based reallocated strings. */
#define va_vec_encode utf8
#endif

#ifndef va_vec16_encode
#include <va_print/malloc_utf16.h>
/** String encoding for 'char16_t' based reallocated strings. */
#define va_vec16_encode utf16
#endif

#ifndef va_vec32_encode
#include <va_print/malloc_utf32.h>
/** String encoding for 'char32_t' based reallocated strings. */
#define va_vec32_encode utf32
#endif

#ifndef va_mprintf_init_size
/** Minimal number of entries in reallocated strings. */
#define va_mprintf_init_size 16
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* macros */

/**
 * Generate an object of type va_stream_vec_t */
#define VA_STREAM_VEC(M) \
    ((va_stream_vec_t){ \
        VA_STREAM(&VA_CONCAT(va_vec_vtab_,va_vec_encode)), NULL, \
        va_mprintf_init_size, 0, (M) })

/**
 * Generate an object of type va_stream_vec16_t */
#define VA_STREAM_VEC16(M) \
    ((va_stream_vec16_t){ \
        VA_STREAM(&VA_CONCAT(va_vec16_vtab_,va_vec16_encode)), NULL, \
        va_mprintf_init_size, 0, (M) })

/**
 * Generate an object of type va_stream_vec32_t */
#define VA_STREAM_VEC32(M) \
    ((va_stream_vec32_t){ \
        VA_STREAM(&VA_CONCAT(va_vec32_vtab_,va_vec32_encode)), NULL, \
        va_mprintf_init_size, 0, (M) })

/**
 * Prints into a newly allocated 'char*' buffer using the given realloc().
 *
 * This doubles the buffer as the string is printed until the string
 * completely fills the buffer.  This starts allocating with a 16 byte
 * string.
 *
 * This returns the allocated string.
 *
 * If memory is exhausted while printing, then the string so far allocated
 * is deallocated using the given realloc() and then NULL is returned.
 */
#define va_mprintf(M,...) \
    VA_BLOCK_EXPR(va_xprintf(&VA_STREAM_VEC(M), __VA_ARGS__)->data)

/**
 * Prints into a newly allocated 'char16_t*' buffer using the
 * given realloc().
 *
 * This doubles the buffer as the string is printed until the string
 * completely fills the buffer.  This starts allocating with a 16 byte
 * string.
 *
 * This returns the allocated string.
 *
 * If memory is exhausted while printing, then the string so far allocated
 * is deallocated using the given realloc() and then NULL is returned.
 */
#define va_umprintf(M,...) \
    VA_BLOCK_EXPR(va_xprintf(&VA_STREAM_VEC16(M), __VA_ARGS__)->data)

/**
 * Prints into a newly allocated 'char32_t*' buffer using the
 * given realloc().
 *
 * This doubles the buffer as the string is printed until the string
 * completely fills the buffer.  This starts allocating with a 32 byte
 * string.
 *
 * This returns the allocated string.
 *
 * If memory is exhausted while printing, then the string so far allocated
 * is deallocated using the given realloc() and then NULL is returned.
 */
#define va_Umprintf(M,...) \
    VA_BLOCK_EXPR(va_xprintf(&VA_STREAM_VEC32(M), __VA_ARGS__)->data)

/* ********************************************************************** */
/* types */

typedef struct {
    va_stream_t s;
    char *data;
    size_t size;
    size_t pos;
    void *(*reall)(void *, size_t);
} va_stream_vec_t;

typedef struct {
    va_stream_t s;
    char16_t *data;
    size_t size;
    size_t pos;
    void *(*reall)(void *, size_t);
} va_stream_vec16_t;

typedef struct {
    va_stream_t s;
    char32_t *data;
    size_t size;
    size_t pos;
    void *(*reall)(void *, size_t);
} va_stream_vec32_t;

/* ********************************************************************** */
/* extern functions */

extern void va_vec_init(
    va_stream_t *);

extern void va_vec_put(
    va_stream_t *,
    char);

extern void va_vec16_init(
    va_stream_t *);

extern void va_vec16_put(
    va_stream_t *,
    char16_t);

extern void va_vec32_init(
    va_stream_t *);

extern void va_vec32_put(
    va_stream_t *,
    char32_t);

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_MALLOC_H_ */
