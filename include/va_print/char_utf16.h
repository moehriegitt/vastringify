/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This prints into char16_t[] arrays using UTF-16 encoding.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_CHAR_UTF16_H_
#define VA_PRINT_CHAR_UTF16_H_

#include <va_print/core.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* extern objects */

extern va_stream_vtab_t const va_char16_p_vtab_utf16;

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_CHAR_UTF16_H_ */
