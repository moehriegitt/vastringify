/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This prints into 'char' based FILE* streams.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_FILE_H_
#define VA_PRINT_FILE_H_

#include <stdio.h>
#include <va_print/core.h>

/* ********************************************************************** */
/* defaults */

#ifndef va_file_encode
#include <va_print/file_utf8.h>
/* String encoding for 'char' based files. */
#define va_file_encode utf8
#endif

#ifndef va_file16_encode
#include <va_print/file_utf16be.h>
/* String encoding for 'char16_t' based files. */
#define va_file16_encode utf16be
#endif

#ifndef va_file32_encode
#include <va_print/file_utf32be.h>
/* String encoding for 'char32_t' based files. */
#define va_file32_encode utf32be
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* macros */

/**
 * Create a 'char' based va_stream_file_t object with an initial
 * format string. */
#define VA_STREAM_FILE(F) \
    ((va_stream_file_t){ \
        VA_STREAM(&VA_CONCAT(va_file_vtab_,va_file_encode)), (F) })

/**
 * Create a 'char16_t' based va_stream_file_t object with an initial
 * format string. */
#define VA_STREAM_FILE16(F) \
    ((va_stream_file_t){ \
        VA_STREAM(&VA_CONCAT(va_file16_vtab_,va_file16_encode)), (F) })

/**
 * Create a 'char32_t' based va_stream_file_t object with an initial
 * format string. */
#define VA_STREAM_FILE32(F) \
    ((va_stream_file_t){ \
        VA_STREAM(&VA_CONCAT(va_file32_vtab_,va_file32_encode)), (F) })

/**
 * Prints a formatted string into a 'char' based FILE*.
 *
 * Returns nothing.
 */
#define va_fprintf(F,...) \
    VA_BLOCK_STMT((void)va_xprintf(&VA_STREAM_FILE(F), __VA_ARGS__))

/**
 * Prints a formatted string into a 'char16_t' based FILE*.
 *
 * Returns nothing.
 */
#define va_ufprintf(F,...) \
    VA_BLOCK_STMT((void)va_xprintf(&VA_STREAM_FILE16(F), __VA_ARGS__))

/**
 * Prints a formatted string into a 'char32_t' based FILE*.
 *
 * Returns nothing.
 */
#define va_Ufprintf(F,...) \
    VA_BLOCK_STMT((void)va_xprintf(&VA_STREAM_FILE32(F), __VA_ARGS__))

/**
 * Prints a formatted string into 'char' based stdout.
 *
 * Returns nothing.
 */
#define va_printf(...) va_fprintf(stdout, __VA_ARGS__)

/**
 * Prints a formatted string into 'char' based stderr.
 *
 * Returns nothing.
 */
#define va_eprintf(...) va_fprintf(stderr, __VA_ARGS__)

/* ********************************************************************** */
/* types */

typedef struct {
    va_stream_t s;
    FILE *file;
} va_stream_file_t;

/* ********************************************************************** */
/* extern functions */

extern void va_file_put(va_stream_t *, char);
extern void va_file16_put_be(va_stream_t *, char16_t);
extern void va_file16_put_le(va_stream_t *, char16_t);
extern void va_file32_put_be(va_stream_t *, char32_t);
extern void va_file32_put_le(va_stream_t *, char32_t);

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_FILE_H_ */
