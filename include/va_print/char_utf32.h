/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This prints into char32_t[] arrays using UTF-32 encoding.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_CHAR_UTF32_H_
#define VA_PRINT_CHAR_UTF32_H_

#include <va_print/core.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* extern objects */

extern va_stream_vtab_t const va_char32_p_vtab_utf32;

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_CHAR_UTF32_H_ */
