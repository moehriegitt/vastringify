/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * This counts codepoints in formatted strings.
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_UTF8_H_
#define VA_PRINT_UTF8_H_

#include <va_print/base.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* extern objects */

extern va_read_iter_vtab_t const va_char_p_read_vtab_utf8;

extern va_read_iter_vtab_t const va_span_p_read_vtab_utf8;

/* ********************************************************************** */
/* extern functions */

/**
 * Decode a single UTF-8 character, stop at NUL, or when iterator is
 * exhausted, and stop at decoding errors and set the the stream error
 * to VA_E_ILSEQ pointer in that case.  When an error is found, it is
 * tried to be skipped (so that there is progress) and
 * VA_U_REPLACEMENT is returned.  At the end of the stream, 0 is
 * returned.
 *
 * If there is an incomplete UTF8 sequence before NUL, then it is output
 * byte by byte with the error marker set.  If an incomplete UTF8
 * sequence is encountered at the end defined by i->size, however, it is
 * assumed that the sequence is correct, just not fully visible, so the
 * function does not advance i->cur and just stops decoding and returns 0.
 *
 * This follows the recommendations of Unicode UTF-8 error handling, and
 * never consumes valid bytes as part of an ill-formed sequences.  Each
 * byte of an ill-formed sequence is indicated separately with the
 * VA_ENC_* error marking (this function does not introduce U+FFFD itself).
 * The use of the VA_ENC_ECONT bit also allows combining errors into a
 * single U+FFFD error marker.
 */
extern unsigned va_char_p_take_utf8(
    va_read_iter_t *i,
    void const *end);

/**
 * Same for arrays (length delimited strings)
 */
extern unsigned va_span_p_take_utf8(
    va_read_iter_t *,
    void const *end);

/**
 * Encode UTF-8 code point.
 *
 * Note: surrogates, values larger than VA_U_MAX, and decoding errors
 * set the stream error VA_E_ENCODE.  Input chars that are marked already
 * as illegal during decoding will be output as is if the input encoding
 * was UTF-8, otherwise illegal chars are output as VA_U_REPLACEMENT,
 * unless the VA_ENC_ECONT bit is set, which suppresses printing of
 * VA_U_REPLACEMENT.  The caller needs to make sure that the VA_ENC_ECONT
 * bit is not set too often (i.e., it must follow the decoder's EMORE
 * hints).
 *
 * This prints the U+0000 as a single byte 0x00.  Printing 0 is used to
 * initialise streams (e.g., to NUL terminate uninitialised arrays or
 * to alloc an initial string).
 *
 * The UTF-8 sequence will be printed into s byte by byte using put(s,b).
 */
extern void va_put_utf8(
    va_stream_t *s,
    unsigned c,
    void (*put)(va_stream_t *, char));

extern va_stream_t *va_xprintf_char_p_utf8(
    va_stream_t *,
    char const *x);

extern va_stream_t *va_xprintf_char_const_pp_utf8(
    va_stream_t *,
    char const **);

extern va_stream_t *va_xprintf_char_pp_utf8(
    va_stream_t *,
    char **);

extern va_stream_t *va_xprintf_last_char_p_utf8(
    va_stream_t *,
    char const *x);

extern va_stream_t *va_xprintf_last_char_const_pp_utf8(
    va_stream_t *,
    char const **);

extern va_stream_t *va_xprintf_last_char_pp_utf8(
    va_stream_t *,
    char **);

extern va_stream_t *va_xprintf_span_p_utf8(
    va_stream_t *,
    va_span_t const *);

extern va_stream_t *va_xprintf_last_span_p_utf8(
    va_stream_t *,
    va_span_t const *);

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_UTF8_H_ */
