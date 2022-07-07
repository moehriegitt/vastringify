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
#define VA_BLOCK_EXPR(A) __extension__({ A ;})

/* macro utils */
#define VA_NIX
#define VA_EAT(...)
#define VA_ECHO(...)    __VA_ARGS__
#define VA_NOC(...)     ,##__VA_ARGS__
#define VA_PAR          ()

/* deep evaluation */
#define VA_EXP1(...)   __VA_ARGS__
#define VA_EXP2(...)   VA_EXP1(VA_EXP1(VA_EXP1(VA_EXP1(__VA_ARGS__))))
#define VA_EXP3(...)   VA_EXP2(VA_EXP2(VA_EXP2(VA_EXP2(__VA_ARGS__))))
#define VA_EXP(...)    VA_EXP3(VA_EXP3(VA_EXP3(VA_EXP3(__VA_ARGS__))))

/* optionality */
#define VA_P(X,...)    X
#define VA_F(...)      VA_H

/* optionality */
#define VA_SUBVA_H(Z,N)  Z
#define VA_SUBVA_F(Z,N)  N
#define VA_OPT3(...)     VA_P(__VA_ARGS__)
#define VA_OPT2(...)     VA_OPT3(VA_SUB ## __VA_ARGS__)
#define VA_OPT1(Z,N,...) VA_OPT2(__VA_ARGS__) (Z,N)
#define VA_OPT0(...)     __VA_ARGS__
#define VA_OPT(Z,N,...)  VA_OPT0(VA_OPT1 VA_NIX (Z,N, VA_F VA_NOC(__VA_ARGS__) ()))

/* recursive expressions*/
#define VA_REC0B(F,A,...)   A
#define VA_REC1B(F,A,X,...) VA_RECA(F,F VA_NIX (VA_OPT(0,1,__VA_ARGS__),A,X),__VA_ARGS__)
#define VA_REC0()           VA_REC0B
#define VA_REC1()           VA_REC1B
#define VA_RECA(F,A,...)    VA_OPT(VA_REC0,VA_REC1,__VA_ARGS__) VA_PAR (F,A,__VA_ARGS__)

#define VA_REC(F,I,A,...)   VA_EXP(VA_RECA(F,I VA_NIX (VA_OPT(0,1,__VA_ARGS__),A),__VA_ARGS__))

/**
 * Compound literal of type va_stream_t.
 */
#define VA_STREAM(F) ((va_stream_t){ F,{0,0},0,0,0,0 })

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

/** Unicode byte order mark */
#define VA_U_BOM 0xfeff

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

/**
 * Return the pointer to a surrounding structure from a pointer to a
 * slot inside that structure.
 *
 * This is complex, because (1) _Target may be an expression or a
 * type, (2) this infers the constness of the pointer and returns the
 * right type, (3) is also robust against passing a const type for
 * _Target, and (4) this checks that the passed struct's slot type is
 * actually compatible with the passed value.
 *
 * Without that magic, this would be a one-liner around offsetof().
 */
#define va_boxof(_val, _Target, _slot) \
    ({ \
        __typeof__(*(_val)) *val_ = (_val); \
        typedef __typeof__(_Target) _Type; \
        _Type *r_ = (_Type*)(((size_t)val_) - offsetof(_Type, _slot)); \
        __auto_type r2_ = _Generic(0 ? val_ : (void*)1, \
            void * : r_, \
            void const * : (_Type const *)r_); \
        __typeof__(r2_->_slot) *val2_ = val_; \
        (void)val2_; \
        r2_; \
    })

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
     * Whether this is a size or NUL terminated string */
    unsigned char has_size;

    /**
     * C string indicator prefix character */
    unsigned char str_prefix;

    char _pad[sizeof(void*)-2];
} va_read_iter_vtab_t;

/**
 * End of stream marker in internal communication if this library.
 *
 *
 * I.e., the value to be returned by a 'va_read_iter_vtab_t::take'
 * function if the end of the string is reached.
 * This is different from 0 so that in size terminated strings
 * and in chars, U+0000 can be handled correctly, and printed.
 *
 * EOT = end of text
 */
#define VA_U_EOT -1U

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

/**
 * A derived read_iter with an additional 'size' argument for
 * the _take function to have an additional end condition.
 */
typedef struct {
    va_read_iter_t super;
    void const *end;
} va_read_iter_end_t;

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
    unsigned qctxt;
};

typedef struct va_print va_print_t;

/**
 * A printer for a custom type.
 *
 * To print a custom type, you can either directly use this
 * type and store a pointer to your value in 'value', or if
 * that's not enough info, you can derive a printer type from
 * this base type into which you encapsulate the value to print.
 * This library will then invoke the print method to print the
 * value.
 *
 * Before invoking \p vtab.print, the \p width, \p prec, and
 * \p opt are set up like in the stream into which this is
 * redirected.
 */
struct va_print {
    void (*print) (va_stream_t *s, va_print_t *);
    void const *value;
    unsigned width;
    unsigned prec;
    unsigned opt;
    unsigned _pad1;
};

/**
 * Constructor for a va_print_t object.
 */
#define VA_PRINT(p_,x_) ((va_print_t){ .print= (p_), .value = (x_) })

/**
 * A length delimited char array for printing of non-NUL terminated strings. */
typedef struct va_span {
    size_t size;
    char const *data;
} va_span_t;

/**
 * A length delimited 32-bit char array for printing of non-NUL terminated strings. */
typedef struct va_span16 {
    size_t size;
    char16_t const *data;
} va_span16_t;

/**
 * A length delimited 32-bit char array for printing of non-NUL terminated strings. */
typedef struct va_span32 {
    size_t size;
    char32_t const *data;
} va_span32_t;

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
 * an allocation failed in realloc() in va_asprintf(). */
#define VA_E_TRUNC 4

/**
 * There was an syntactic error in a format specification. */
#define VA_E_FORMAT 5

/**
 * Too many or too few arguments were given for the format string. */
#define VA_E_ARGC 6

/**
 * The EACH() macro applied to all VA_E_* error codes. */
#define VA_E_FOREACH \
    EACH(VA_E_OK) \
    EACH(VA_E_NULL) \
    EACH(VA_E_DECODE) \
    EACH(VA_E_ENCODE) \
    EACH(VA_E_TRUNC) \
    EACH(VA_E_FORMAT) \
    EACH(VA_E_ARGC)

/* ********************************************************************** */
/* epilogue */

#ifdef __cplusplus
}
#endif

#endif /* VA_PRINT_BASE_H_ */
