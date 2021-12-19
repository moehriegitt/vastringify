/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This prints into 'char' based FD streams using UTF-8 encoding.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_FD_UTF8_H_
#define VA_PRINT_FD_UTF8_H_

#include <va_print/core.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* extern objects */

extern va_stream_vtab_t const va_fd_vtab_utf8;

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_FD_UTF8_H_ */
