/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This counts codepoints in formatted strings.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_LEN_H_
#define VA_PRINT_LEN_H_

#include <va_print/core.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* macros */

/**
 * Generate a value of typeva_stream_len_t */
#define VA_STREAM_LEN() \
    ((va_stream_len_t){ VA_STREAM(&va_len_vtab), 0 })

/**
 * Compute length of printed string, without actually
 * printing.
 *
 * This returns the length of the printed string, which is
 * one less than the size needed for a char array to fit the
 * string.
 */
#define va_lprintf(...) \
    (va_xprintf(&VA_STREAM_LEN(), __VA_ARGS__)->pos)

/* ********************************************************************** */
/* types */

typedef struct {
    va_stream_t s;
    size_t pos;
} va_stream_len_t;

/* ********************************************************************** */
/* extern objects */

extern va_stream_vtab_t const va_len_vtab;

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_LEN_H_ */
