/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This counts codepoints in formatted strings.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_UTF16_H_
#define VA_PRINT_UTF16_H_

#include <uchar.h>
#include <va_print/base.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* extern functions */

extern unsigned va_char16_p_take_utf16(
    va_read_iter_t *);

/**
 * Encode UTF-16 code point.
 *
 * Note: surrogates and values larger than VA_U_MAX set the stream
 * error VA_E_ENCODE.  Input chars that are marked already
 * as illegal during decoding will be output as is if the input encoding
 * was UTF-32, otherwise illegal chars are output as VA_U_REPLACEMENT.
 *
 * This prints the U+0000 as 0x0000.  Printing 0 is used to
 * initialise streams (e.g., to NUL terminate uninitialised arrays or
 * to malloc an initial string).
 *
 * The UTF-16 sequence will be printed into s word by word using put(s,w).
 */
extern void va_put_utf16(
    va_stream_t *,
    unsigned,
    void (*put)(va_stream_t *, char16_t));

extern va_stream_t *va_xprintf_char16_p_utf16(
    va_stream_t *,
    char16_t const *);

extern va_stream_t *va_xprintf_char16_const_pp_utf16(
    va_stream_t *,
    char16_t const **);

extern va_stream_t *va_xprintf_char16_pp_utf16(
    va_stream_t *,
    char16_t **);

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_UTF16_H_ */
