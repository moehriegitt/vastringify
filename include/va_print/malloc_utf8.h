/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This prints into dynamically allocated 'char' based vectors.arrays
 * that are reallocated as printing proceeds to accomodate the whole
 * string.  It uses UTF-8 encoding.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_MALLOC_UTF8_H_
#define VA_PRINT_MALLOC_UTF8_H_

#include <va_print/core.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* extern objects */

extern va_stream_vtab_t const va_vec_vtab_utf8;

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_MALLOC_UTF8_H_ */
