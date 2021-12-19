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
#include <va_print/alloc_utf8.h>
/** String encoding for 'char' based reallocated strings. */
#define va_vec_encode utf8
#endif

#ifndef va_vec16_encode
#include <va_print/alloc_utf16.h>
/** String encoding for 'char16_t' based reallocated strings. */
#define va_vec16_encode utf16
#endif

#ifndef va_vec32_encode
#include <va_print/alloc_utf32.h>
/** String encoding for 'char32_t' based reallocated strings. */
#define va_vec32_encode utf32
#endif

#ifndef va_asprintf_init_size
/** Minimal number of entries in reallocated strings. */
#define va_asprintf_init_size 16
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
        va_asprintf_init_size, 0, (M) })

/**
 * Generate an object of type va_stream_vec16_t */
#define VA_STREAM_VEC16(M) \
    ((va_stream_vec16_t){ \
        VA_STREAM(&VA_CONCAT(va_vec16_vtab_,va_vec16_encode)), NULL, \
        va_asprintf_init_size, 0, (M) })

/**
 * Generate an object of type va_stream_vec32_t */
#define VA_STREAM_VEC32(M) \
    ((va_stream_vec32_t){ \
        VA_STREAM(&VA_CONCAT(va_vec32_vtab_,va_vec32_encode)), NULL, \
        va_asprintf_init_size, 0, (M) })

/**
 * Prints into a newly allocated 'char*' buffer using the given alloc()
 * function.
 *
 * The function uses the passed alloc() function for initial allocation
 * by passing a NULL pointer, for reallocation, and also for deallocation
 * by passing nmemb==0, in case of an error.  Do not pass the system's
 * 'realloc()' function directly, as it is not well-defined for the
 * 'nmemb==0' case.
 *
 * This doubles the buffer as the string is printed until the string
 * completely fills the buffer.  This starts allocating with a 16 byte
 * string.
 *
 * This returns the allocated string.
 *
 * If memory is exhausted while printing, then the string so far allocated
 * is deallocated using the given 'alloc(p,0)' and then NULL is returned.
 *
 * Note: to prevent that you pass the system 'realloc()' function direction,
 * the prototype of the function was made incompatible on purpose.
 * Using realloc() directly is not possible with the current state of the C
 * standard, because it is implementation defined how it works when nmemb==0:
 * it may not deallocate the memory.  There is the va_alloc() function that
 * uses the system 'realloc()' and 'free()' to implement the 'nmemb==0'
 * deallocation.  If you pass own functions, be sure that they can
 * allocate (ptr==NULL), reallocate, and deallocate (nmemb==0).
 */
#define va_axprintf(alloc,...) \
    VA_BLOCK_EXPR(va_xprintf(&VA_STREAM_VEC(alloc), __VA_ARGS__)->data)

/**
 * Prints into a newly allocated 'char16_t*' buffer using the
 * given allocator function.
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
#define va_uaxprintf(M,...) \
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
#define va_Uaxprintf(M,...) \
    VA_BLOCK_EXPR(va_xprintf(&VA_STREAM_VEC32(M), __VA_ARGS__)->data)

/**
 * The va_axprintf() function used with va_alloc(), i.e., with the
 * system `realloc` and `free` allocator.
 */
#define va_asprintf(...) va_axprintf(va_alloc, __VA_ARGS__)

/**
 * The va_uaxprintf() function used with va_alloc(), i.e., with the
 * system `realloc` and `free` allocator.
 */
#define va_uasprintf(...) va_uaxprintf(va_alloc, __VA_ARGS__)

/**
 * The va_Uaxprintf() function used with va_alloc(), i.e., with the
 * system `realloc` and `free` allocator.
 */
#define va_Uasprintf(...) va_Uaxprintf(va_alloc, __VA_ARGS__)

/* ********************************************************************** */
/* types */

typedef struct {
    va_stream_t s;
    char *data;
    size_t size;
    size_t pos;
    void *(*alloc)(void *, size_t nmemb, size_t size);
} va_stream_vec_t;

typedef struct {
    va_stream_t s;
    char16_t *data;
    size_t size;
    size_t pos;
    void *(*alloc)(void *, size_t nmemb, size_t size);
} va_stream_vec16_t;

typedef struct {
    va_stream_t s;
    char32_t *data;
    size_t size;
    size_t pos;
    void *(*alloc)(void *, size_t nmemb, size_t size);
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

/**
 * Allocate, reallocate, or deallocate.
 *
 * If nmemb==0 this uses free() to deallocate the memory returns NULL.
 * Note that free() also allows data to be NULL in this case.
 *
 * If nmemb>0, this tries to allocate or reallocate a new piece of
 * memory using realloc(), and returns the result.
 *
 * This asserts that size != 0: this parameter should come from a
 * sizeof() operator.
 *
 * The string returned by vm_asprintf() when v_alloc() is used can be
 * freed using 'free()'.  No need to use 'va_alloc()' for freeing
 * (and thinking about the value of 'size').
 */
extern void *va_alloc(
    void *data,
    size_t nmemb,
    size_t size);

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_MALLOC_H_ */
