/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This prints into 'char' based FD streams.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_FD_H_
#define VA_PRINT_FD_H_

#include <va_print/core.h>

/* ********************************************************************** */
/* defaults */

#ifndef va_fd_encode
#include <va_print/fd_utf8.h>
/* String encoding for 'char' based FDs. */
#define va_fd_encode utf8
#endif

#ifndef va_fd16_encode
#include <va_print/fd_utf16be.h>
/* String encoding for 'char16_t' based FDs. */
#define va_fd16_encode utf16be
#endif

#ifndef va_fd32_encode
#include <va_print/fd_utf32be.h>
/* String encoding for 'char32_t' based FDs. */
#define va_fd32_encode utf32be
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* macros */

/**
 * Create a 'char' based va_stream_fd_t object with an initial
 * format string. */
#define VA_STREAM_FD(FD) \
    ((va_stream_fd_t){ \
        VA_STREAM(&VA_CONCAT(va_fd_vtab_,va_fd_encode)), (FD) })

/**
 * Create a 'char16_t' based va_stream_fd_t object with an initial
 * format string. */
#define VA_STREAM_FD16(FD) \
    ((va_stream_fd_t){ \
        VA_STREAM(&VA_CONCAT(va_fd16_vtab_,va_fd16_encode)), (FD) })

/**
 * Create a 'char32_t' based va_stream_fd_t object with an initial
 * format string. */
#define VA_STREAM_FD32(FD) \
    ((va_stream_fd_t){ \
        VA_STREAM(&VA_CONCAT(va_fd32_vtab_,va_fd32_encode)), (FD) })

/**
 * Prints a formatted string into a 'char' based FD.
 *
 * Returns nothing.
 */
#define va_dprintf(F,...) \
    VA_BLOCK_STMT((void)va_xprintf(&VA_STREAM_FD(F), __VA_ARGS__))

/**
 * Prints a formatted string into a 'char16_t' based FD.
 *
 * Returns nothing.
 */
#define va_udprintf(F,...) \
    VA_BLOCK_STMT((void)va_xprintf(&VA_STREAM_FD16(F), __VA_ARGS__))

/**
 * Prints a formatted string into a 'char32_t' based FD.
 *
 * Returns nothing.
 */
#define va_Udprintf(F,...) \
    VA_BLOCK_STMT((void)va_xprintf(&VA_STREAM_FD32(F), __VA_ARGS__))

/* ********************************************************************** */
/* types */

typedef struct {
    va_stream_t s;
    long fd;
} va_stream_fd_t;

/* ********************************************************************** */
/* extern functions */

extern void va_fd_put(va_stream_t *, char);
extern void va_fd16_put_be(va_stream_t *, char16_t);
extern void va_fd16_put_le(va_stream_t *, char16_t);
extern void va_fd32_put_be(va_stream_t *, char32_t);
extern void va_fd32_put_le(va_stream_t *, char32_t);

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_FD_H_ */
