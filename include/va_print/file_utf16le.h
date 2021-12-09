/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This prints into 'char' based FILE* streams using UTF-16LE encoding.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_FILE_UTF16LE_H_
#define VA_PRINT_FILE_UTF16LE_H_

#include <stdio.h>
#include <va_print/core.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* extern objects */

extern va_stream_vtab_t const va_file16_vtab_utf16le;

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_FILE_UTF16LE_H_ */
