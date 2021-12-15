/* -*- Mode: C -*- */
/* (c) Henrik Theiling, LICENSE: BSD-3-Clause */

/**
 * Basic definitions, but no function declarations
 */

/* ********************************************************************** */
/* prologue */

#ifndef VA_PRINT_BASE_H_
#define VA_PRINT_BASE_H_

#include <stddef.h>
#include <stdbool.h>
#include <uchar.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************** */
/* macros */

/* misc */
#define VA_CONCAT(A,B) VA_CONCAT1(A,B)
#define VA_CONCAT1(A,B) VA_CONCAT2(A,B)
#define VA_CONCAT2(A,B) A ## B

#define VA_BLOCK_STMT(A) do{ A ;}while(0)
#define VA_BLOCK_EXPR(A) ({ A ;})

/* deep evaluation */
#define VA_EXP1(...)   __VA_ARGS__
#define VA_EXP2(...)   VA_EXP1(VA_EXP1(VA_EXP1(VA_EXP1(__VA_ARGS__))))
#define VA_EXP3(...)   VA_EXP2(VA_EXP2(VA_EXP2(VA_EXP2(__VA_ARGS__))))
#define VA_EXP(...)    VA_EXP3(VA_EXP3(VA_EXP3(VA_EXP3(__VA_ARGS__))))

/* optionality */
#define VA_NIX
#define VA_PAR ()
#define VA_P(X,...)    X
#define VA_F(...)      VA_H
#define VA_O3(...)     VA_P(__VA_ARGS__)
#define VA_O2(SUB,...) VA_O3(SUB ## __VA_ARGS__)
#define VA_O1(SUB,...) VA_O2(SUB,__VA_ARGS__)
#define VA_O(SUB,...)  VA_O1(SUB, VA_F ,##__VA_ARGS__ ())

/* recursive expressions */
#define VA_RECVA_F     VA_RECC1,
#define VA_RECVA_H     VA_RECC0,
#define VA_RECC0()     VA_RECC00
#define VA_RECC1()     VA_RECB2

#define VA_RECC00(FUN,ARG,...) ARG

#define VA_RECB2(FUN,A1,A2,...) \
    VA_RECB(FUN,FUN(A1,A2),##__VA_ARGS__)

#define VA_RECB(FUN,ARG,...)  \
    VA_O(VA_REC,##__VA_ARGS__) VA_PAR (FUN,ARG,##__VA_ARGS__)

#define VA_NOC(...) ,##__VA_ARGS__
#define VA_REC(FUN,ARG,...)  \
    VA_EXP(VA_RECB VA_NIX (FUN,ARG VA_NOC(__VA_ARGS__)))

/**
 * Compound literal of type va_stream_t.
 */
#define VA_STREAM(F) ((va_stream_t){ F,{0,0},0,1,0,0 })

/** Iterator for extracting single codepoint data */
#define VA_READ_ITER(TAKE,DATA) \
    ((va_read_iter_t){ (TAKE),(DATA) })

/** Mask for marking enconding, also on extracted characters.
 *
 * This marks the encoding in which the given character was
 * invalid.  See va_read_iter_t.
 */
#define VA_U_ENC 0x1f000000

/**
 * Mask for extracting the data from a value returned by va_read_iter_t::take
 * in case there is an encoding error.
 */
#define VA_U_DATA 0x00ffffff

/**
 * If an error flag is set, how many following errors should be
 * combined into a single \xfffd.  There can be max. 3 continued
 * error bits.
 * The decoder will return each input code unit as a single error,
 * it will never combine sequences into one error, so that the
 * erroneous sequence could be mirrored by an output encoder.
 * The decoder also has no storage for state, so it may not recognise
 * the following code units as errors (UTF-8 is constructed in such a
 * way that that can be recognised as an error, but this prepares
 * for cases where this is not the case).  The user of the decoder
 * needs to store those three bits of state and count how many code
 * units follow and turn them into errors, too, or alternatively output
 * a single VA_U_REPLACEMENT for the whole sequence.
 */
#define VA_U_EMORE (29,3U)

/**
 * Whether this is a suspected continued error -- the printer
 * resets this if the previous EMORE setting indicates that this
 * is not a continue error anymore.
 */
#define VA_U_ECONT 0x80000000

/** Erroneous character, unknown encoding. */
#define VA_U_ENC_ERR VA_U_ENC

/** UTF-8 encoding */
#define VA_U_ENC_UTF8 0x01000000

/** UTF-16 encoding */
#define VA_U_ENC_UTF16 0x02000000

/** UTF-32 encoding */
#define VA_U_ENC_UTF32 0x03000000

/** ISO-8859-1 encoding */
#define VA_U_ENC_ISO88591 0x04000000

/* Unicode standard values */

/** Unicode replacement character */
#define VA_U_REPLACEMENT 0xfffd

/** Unicode object replacement character */
#define VA_U_OBJECT 0xfffc

/** Unicode maximum codepoint */
#define VA_U_MAX 0x10ffff

/** This library's internal maximum codepoint value */
#define VA_U_MAXMAX VA_U_DATA

/** Unicode first surrogate */
#define VA_U_SURR_MIN 0xd800

/** Unicode last surrogate */
#define VA_U_SURR_MAX 0xdfff

/* misc */

/** get the size of an array, counting the number of elements */
#define va_countof(A) (sizeof(A)/sizeof((A)[0]))

/* ********************************************************************** */
/* types */

struct va_read_iter;

/**
 * String reader methods */
typedef struct {
    /**
     * The name of the type */
    char const *type;

    /**
     * Decodes one Unicode character from memory pointed to
     * by 'data', returns it, and advances 'data' by the
     * amount read.  It never decodes past a NUL terminator.
     *
     * It never decodes past 'end'.  If the sequence is
     * incomplete because 'end' is reached or NUL is read,
     * then 'data' is not advanced and 0 is returned.
     *
     * 'end' may be set to NULL to indicate that no end is
     * given for the string, but only NUL termination terminates
     * the string.
     */
    unsigned (*take)(struct va_read_iter *, void const *end);

    /**
     * Compute the end of an array */
    void const *(*end)(struct va_read_iter *, size_t size);

    /**
     * C string indicator prefix character */
    unsigned char str_prefix;

    char _pad[sizeof(void*)-1];
} va_read_iter_vtab_t;


/**
 * String reader type.
 *
 * The 'take' function should return a Unicode code point decoded from
 * whatever underlying encoding is used.  If there is an encoding error,
 * the original value should be returned with its VA_ENC_* value ORed
 * (that's in the upper 8 bits and outside of any bit used by Unicode
 * codepoints).  This allows the printer to either copy the faulty
 * encoding, if the output encoding is identical, or to use
 * VA_U_REPLACEMENT to indicate the error.  If the decoder is unable
 * to correctly return the faulty sequence this way (e.g. because it would
 * need state, which is not available here), it should use VA_ENC_ERR
 * for the encoding andskip the faulty sequence, which in turn will be
 * recognised and replaced by VA_REPLACMENT_CHAR by the printer.
 * Any VA_ENC_* return value causes the printer to set an VA_E_ILLDEC
 * error on the stream.
 *
 * If there is an incomplete character at the end of the sequence,
 * then this should not be half-decoded as illegal sequence, but
 * should instead be left in place and 0 should be returned to mark
 * the end of the sequence.  The 'data' pointer should be left
 * pointing to the first data of the incomplete sequence.
 *
 * 'Printing' a pointer to a pointer to char, short, or unsigned will
 * update the given string pointer to point to the next element of
 * the sequence.  This can be used to see how many bytes/words where
 * actually taken from the string.
 *
 * 'Printing' a va_read_iter_t* will print data extracted from that
 * iterator.  Note that it must be possible to reset the iterator
 * by copying the data structure, i.e., 'take' cannot just read from
 * unseekable streams, because the string printer needs to count
 * and possibly analyse the stream for unprintable characters before
 * actually printing, i.e., it may be iterated multiple times.
 */
typedef struct va_read_iter {
    /**
     * Methods */
    va_read_iter_vtab_t const *vtab;
    /**
     * The next character to be decoded. */
    void const *cur;
} va_read_iter_t;

typedef struct va_stream va_stream_t;

/**
 * Stream methods.
 *
 * OK, this is getting a bit like C++, sorry.  We need different
 * functions to print into a stream.
 */
typedef struct {
    /** initialises the stream */
    void (*init)(va_stream_t *);
    /** puts one code point into the output stream */
    void (*put)(va_stream_t *, unsigned c);
} va_stream_vtab_t;

/**
 * Stream: management type for printing.
 *
 * The 'c' passed to 'put' is either a valid Unicode codepoint, or a
 * decoding error.  Decoding errors are marked with the encoding during
 * decoding in the VA_ENC_MASK bits, so the the encoder can check
 * whether this matches the output encoding.  In this case, the
 * lower bits of 'c' should be printed as is, i.e., the encoding
 * error should be passed through.  If the encoder does not know
 * how to deal with this, it should instead output
 * VA_U_REPLACEMENT.  See va_read_iter_t for more info on how
 * the reader encodes read errors.
 *
 * For errors that are from the same encoding sequence (e.g.,
 * if there is an encoded UTF-16 surrogate in a three-byte UTF-8
 * sequence), the number of erroneous code units that follow
 * can be marked using VA_ENC_EMORE, so that the encoder can
 * choose to only output a single VA_U_REPLACEMENT.  E.g.,
 * the UTF-8 decoder sets VA_ENC_EMORE to 0,1, or 2.
 */
struct va_stream {
    va_stream_vtab_t const *vtab;
    va_read_iter_t pat;
    unsigned width;
    unsigned prec;
    unsigned opt;
    unsigned _opt2;
};

/* error handling */

/**
 * Type to represent the error status of the printing.
 *
 * 'Printing' a pointer to this type causes the current error to be
 * written through that pointer, and be reset to OK.  Putting this
 * last on a command line gets the overalll error state.
 *
 * This type is not an integer or enum so that there is no _Generic()
 * ambiguity.
 */
typedef struct {
    unsigned code;
} va_error_t;

/**
 * everything is fine */
#define VA_E_OK 0

/**
 * A string was NULL and was printed as empty. */
#define VA_E_NULL 1

/**
 * decoding the input string encountered an illegal encoding sequence */
#define VA_E_DECODE 2

/**
 * encoding something that the encoder cannot handle will cause this
 * error to be set. */
#define VA_E_ENCODE 3

/**
 * Output stream error: the output is truncated due to an error: the printer hit
 * the end of the statically sized array, or the file printer had an error, or
 * an allocation failed in realloc() in va_mprintf(). */
#define VA_E_TRUNC 4

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_BASE_H_ */
