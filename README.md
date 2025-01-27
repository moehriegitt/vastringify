# Type-Safe Printf For C

This uses macro magic, compound literals, and _Generic to take
printf() to the next level: type-safe printing, printing into compound
literal char arrays, easy UTF-8, -16, and -32, with good error
handling.

The goal is to be safe by removing the need for function varargs.

The usual C printf formatting syntax is used, with some restrictions
and quite a few extensions.

This let's you mix UTF-8, -16, -32 strings seamlessly in input and
output strings, without manual string format conversions, and without
using different format specifiers or print function names.

This liberates you from thinking about `%u` vs. `%lu` vs. `%llu`
vs. `%zu`, even in portable code with different integer types. .  With
this library, the compiler chooses the right function for your
parameter and they all print fine with `~s`, like also strings and
pointers do.

'Type-safe' in this context does not mean that you get more compile
errors or warnings, but that you cannot make a mistake, and you do not
need the format string to specify the argument type.  Format strings
with this library do not need compile-time checking to be safe,
because the compiler chooses the right formatting function for each
parameter.  You cannot pass the wrong size parameter and crash,
because `...` is avoided.

## Examples

 - `va_printf("~s ~s ~s", 65, (long long)65, "65")` prints `65 65 65`
 - `va_printf("~c ~c ~c", 65, (long long)65, "65")` prints `A A 65`
 - `va_printf("~x ~x ~x", 65, (long long)65, "65")` prints `41 41 65`
 - `va_printf("~p ~p ~p", 65, (long long)65, "65")` prints
   `0x41 0x41 0x8838abc3932` (the pointer value of the string)
 - `va_lprintf("~p", 65)` returns `4`, the length of `0x41`
 - `va_nprintf(10, "~p", 65)` returns `"0x41"`, the pointer to a compound
   literal `(char[10]){}` that was printed into
 - `va_printf("~t x = ~=qzs", u"fo\020o")` prints `char16_t* x = u"fo\no"`

## Compatibility

This library requires at least a C11 compiler (for `_Generic`,
`char16_t`, `char32_t`), and it uses a few gcc extensions that are
also understood by Clang and a few other compilers (`({...})`,
`,##__VA_ARGS__`, `__typeof__`, `__attribute__`).

## Synopsis

In the following, `Char` may be `char`, `char16_t`, or `char32_t`:

```c
#include <va_print/file.h>

void
va_fprintf(FILE *f, Char const *format, ...);

void
va_ufprintf(FILE *f, Char const *format, ...);

void
va_Ufprintf(FILE *f, Char const *format, ...);

void
va_printf(Char const *format, ...);

void
va_eprintf(Char const *format, ...);

va_stream_file_t
VA_STREAM_FILE(FILE *f);


#include <va_print/char.h>

Char *
va_snprintf(Char *s, size_t n, Char const *format, ...);

Char *
va_sprintf(Char s[], Char const *format, ...);

char *
va_nprintf(size_t n, Char const *format, ...);

char16_t *
va_unprintf(size_t n, Char const *format, ...);

char32_t *
va_Unprintf(size_t n, Char const *format, ...);

CharType *
va_gnprintf(CharType, size_t n, Char const *format, ...);

size_t
va_zprintf(Char const *format, ...);

size_t
va_uzprintf(Char const *format, ...);

size_t
va_Uzprintf(Char const *format, ...);

size_t
va_gzprintf(CharType, Char const *format, ...);

va_stream_charp_t
VA_STREAM_CHAR_P(Char *s, size_t n);


#include <va_print/alloc.h>

char *
va_axprintf(void *(*alloc)(void *, size_t, size_t), Char const *, ...);

char16_t *
va_uaxprintf(void *(*alloc)(void *, size_t, size_t), Char const *, ...);

char32_t *
va_Uaxprintf(void *(*alloc)(void *, size_t, size_t), Char const *, ...);

char *
va_asprintf(Char const *, ...);

char16_t *
va_uasprintf(Char const *, ...);

char32_t *
va_Uasprintf(Char const *, ...);

va_stream_vec_t
VA_STREAM_VEC(void *(*alloc)(void *, size_t, size_t));

va_stream_vec16_t
VA_STREAM_VEC16(void *(*alloc)(void *, size_t, size_t));

va_stream_vec32_t
VA_STREAM_VEC32(void *(*alloc)(void *, size_t, size_t));

void *
va_alloc(void *data, size_t nmemb, size_t size);


#include <va_print/fd.h>

void
va_dprintf(int fd, Char const *format, ...);

void
va_udprintf(int fd, Char const *format, ...);

void
va_Udprintf(int fd, Char const *format, ...);

va_stream_file_t
VA_STREAM_FD(int fd);


#include <va_print/len.h>

size_t
va_lprintf(Char const *format, ...);

va_stream_len_t
VA_STREAM_LEN();


#include <va_print/core.h>

va_stream_...t *
va_xprintf(va_stream_...t *s, Char const *format, ...);

void
va_iprintf(va_stream_...t *s, Char const *format, ...);

void
va_pprintf(va_stream_vtab_t *v, Char const *format, ...);

unsigned
va_stream_get_error(va_stream_...t const *s);

extern
char const *va_strerror(unsigned error_code);


#include <va_print/base.h>

typedef struct { ... } va_stream_t;

typedef struct { ... } va_stream_vtab_t;

typedef struct { unsigned code; } va_error_t;
#define VA_E_OK     ...
#define VA_E_NULL   ...
#define VA_E_DECODE ...
#define VA_E_ENCODE ...
#define VA_E_TRUNC  ...
#define VA_E_FORMAT ...
#define VA_E_ARGC   ...

va_stream_t
VA_STREAM(va_stream_vtab_t const *vtab);

#define VA_U_REPLACEMENT 0xfffd
#define VA_U_BOM         0xfeff
#define VA_U_SURR_MIN    0xd800
#define VA_U_SURR_MAX    0xdfff
#define VA_U_MAX         0x0010ffff
#define VA_U_MAXMAX      0x00ffffff

#define va_countof(A)    (sizeof(A)/sizeof((A)[0]))
```

## Description

This library provides a type-safe printing mechanism to print
any kind of string of base type `char`, `char16_t`, or `char32_t`,
or any integer or pointer into a new string, an array, or a file.

The library also provides functions for user-defined output streams
that can print into any other kind of stream.

The arguments to the formatted print are passed into a `_Generic()`
macro instead of '...' and the resulting function call is thus
type-safe based on the actual argument type, and cannot crash due to a
wrong format specifier.

The format specifiers in this printing mechanism serve to define which
output format should be used, as they are not needed for type
information.  The format specifier "~s" can be used as a generic
'default' output format.

### Format Specifiers

The format is string is decoded and then output as is into the output
stream where it is encoded, except for sequences of `~`, which are
interpreted as a format specifier.

For each format specifier in the format string, 0, 1, or more
arguments are read (how many is specified below).  If there are more
arguments than what is needed for the format specifiers, the rest of
the argumnets are ignored and the `VA_E_ARGC` stream error is set.

If fewer arguments are given than needed for the format string, the
rest of the format specifiers print empty and the `VA_E_ARGC` stream
error is set.

A format specifier begins with `~`, and what follows is similar to C:

 - a list of flag characters
 - a width specifier
 - a precision specifier
 - a list of integer mask and quotation specifiers
 - a conversion letter

`~` is used instead of `%` to avoid confusion in source code that uses
both this library and the standard C `printf`.

#### Flags

Generally, specifying multiple identical flags like `~008s` is
reserved for future use and should be avoided. It is unspecified how
the current library handles such format strings.

 - `#` print in alternative form.  For numeric format, a prefix to
   designate the base is prefixed to the value except to 0:
     - for `o` and base 8, `0` is prefixed
     - for `b` and base 2, `0b` is prefixed,
     - for `B` and base 2, `0B` is prefixed,
     - for `x` and base 16, `0x` is prefixed,
     - for `X` and base 16, `0X` is prefixed,
     - for `e` and base 32, `0e` is prefixed,
     - for `E` and base 32, `0E` is prefixed.

   In `~#p`, the `#` flag switches off the implicit `#` that is
   contained in `p`, e.g., does not print the base prefix.

   For quoted strings, `#` inhibits printing of delimiting quotes.

 - `0` pads numerics with zero `0` on the left rather than
   with a space character ` `.  If a precision is given, this is
   ignored.

   For C and JSON quotation, this selects to quote non-US-ASCII
   characters using `\u` and `\U` instead of printing them in
   output encoding.

 - `-` selects to left flush instead of the default right flush.

 - ` ` (a space character U+0020) selects that a space is printed in
   front of positive signed integers. Nothing is printed if the
   precision is 0 and the value is 0 (this is different compared to
   the behaviour of C's printf).

 - `+` selects that a `+` is printed in front of positive signed
   integers and zero.  Nothing is printed if the precision is 0 and
   the value is 0 (this is different compared to the behaviour of C's
   printf).

 - `=` specifies that the last value is printed again using this
   new format specifier.  This is meager replacement for the `$`
   position specifiers that are not implemented in this library.

A width is either a decimal integer, or a `*`.  The `*` selects
that the width is taken from the next function parameter.  If fewer
code points result from the conversion, the output is padded with
white space up the width.  A negative width is intepreted as
a `-` flag followed by a positive width.

A precision is specified by a `.` (period) followed by either a
decimal integer or a `*`.  The `*` selects that the width is taken
from the next function parameter.  If the precision is just `.`,
it is interpreted as zero.  The precision defines the minimum number
of digits in numeric conversions. For strings, this is the maximum
number of raw code units read from the input string (not the number
of converted code points, but the low-level number of elements
in the string, so that non-NUL terminated arrays can be printed
with their size passed as precision, even with multi-byte/multi-word
encodings stored inside.  Alternatively, there is `va_span_t` for a
string prefix parameter type.

#### End of String and Encoding Errors

The input decoder (e.g. for UTF-8) has two modes of operation,
distinguished by how incomplete sequences at the end of the input
stream are handled.

In normal mode, the input decoder reads every byte up until the last
one from the input stream.  Incomplete sequences at the end will be
reported as decoding errors.

In chunk mode, the input decoder stops before an incomplete sequence
at the end, simply reporting the end of text.  Incomplete sequences
will not be iterated into, so that the stream can be printed
chunk-wise even if encoding sequences cross the boundary between two
chunks.

The mode is selected by the input type that is printed.  Normal mode
is used by default.  To select chunk mode, an iterator
(`va_read_iter_t`) or a pointer to a string (`char const **`) needs to
be printed.  This is because only for iterators, the printer can
report back how much data was read, which is required for chunk mode
anyway, to know where to start to print the next chunk (which is where
the decoder stopped for the previous chunk).  The caller can read the
decoder stop position from the iterator.

Setting up the `va_read_iter_t` manually is a bit of a mouthful, but
it is not needed frequently, I presume, so there is no syntactic sugar
for it.  E.g. to print a chunk of bytes from a `char* s` in UTF-8
format in chunk mode, i.e., stopping before an incomplete sequence at the end,
the following code can be used:

```c
va_read_iter_t iter = VA_READ_ITER(&va_char_p_read_vtab_utf8, s);
va_eprintf("~s", &iter);
```
After that, `iter.cur` can be checked to see how many bytes were read.

#### Integer Mask and Quotation Specifiers

Generally, specifying multiple identical mask and quotation specifiers
or more than listed in the following list, like `~zzu` or `~hhhx`, is
reserved for future use and should be avoided.  It is unspecified how
the current library handles such format strings.

 - `h` applies the mask `0xffff` to an integer, then zero extends unsigned
   values, or sign extends signed values.
   E.g., `va_printf("~#hx",0xabcdef)` prints `-0x3211`.

 - `hh` applies the mask `0xff` to an integer, then zero extends unsigned
   values, or sign extends signed values.
   E.g., `va_printf("~hhX",0xabcdU)` prints `CD`.

 - `z` reinterprets a signed integer as unsigned (mnemonic: zero
   extension).  `z` is implicit in formats `u` (and `U`).
   E.g., `va_printf("~hhu", -1)` prints `255`.

 - `q` selects C quotation for strings and char format.  There is a
   separate section below to explain this.

 - `Q` selects JSON quotation for strings and char format.  There is a
   separate section below to explain this.

 - `k` selects Bourne or Korn shell quotation.  There is a
   separate section below to explain this.

 - `K` additional custom quotation

 - `qq`, `QQ`, `kk` more additional custom quotations

Note that most of the usual length specifiers (`l`, `ll`, etc.) known
from C make no sense and are not recognised (nor ignored), because
type casting control in varargs is not needed here due to the
type-safety.

#### Conversions

The format specifiers is terminated by a single conversion character
from the following list.

 - `s` prints anything in default notation (mnemonic: 'standard').
   `s` is used by standard C `printf` for strings, and CommonLisp
   uses `~s` for 'standard' format.

 - `o` selects octal integer notation for numeric printing (including
   pointers).

 - `d` or `i` selects decimal integer notation for numeric
   printing (including pointers).

 - `u` is equivalent to `zd`, i.e., prints a signed integer as
   unsigned in decimal notation.  This implicitly sets the `z`
   option, which also affects quoted string printing, so `~qu`
   prints strings like `~qzs`.

 - `x` or `X` selects hexadecimal integer notation for numeric
   printing (including pointers).  `x` uses lower case digits,
   `X` upper case.  Note that this also prints signed numbers with
   a `-` if appropriate: `va_printf("~#x", -5)` prints `-0x5`.
   There's the `z` flag to print signed integers as unsigned.

 - `b` or `B` selects binary integer notation for numeric
   printing (including pointers).  `b` uses lower case prefix,
   `B` uses upper case.  The difference is only visible
   with the `#` flag.

 - `e` or `E` selects Base32 notation using the digits
   'a'..'z','2'..'7'.  `e` uses lower case digits and prefix,
   `E` uses upper case.

 - `p` prints like `x`, toggles the `#` flag, and for any strings,
   prints the pointer value instead of the contents.  Note that
   it also prints signed numbers: `va_print("~p",-5)` prints `-0x5`.

   Note that `~#p` prints pointers like `~x` and `~p` prints like
   `~#x`, i.e., the `#` flag is toggled.

 - `c` prints integers (but not pointers) as characters, like a
   one-element string.  Note that the NUL character is not printed,
   but behaves like an empty string, unless quotation is used.
   For string quotation where hexadecimals are printed, this uses
   lower case characters.

 - `a`, `f`, and `g` print like `s`, but will print differently when
   floating point support is added.

 - `t` prints the argument type in C syntax:
   `int8_t`..`int64_t`, `uint8_t`..`uint64_t`, `char*`,
   `char16_t*`, `char32_t*`, `void*`.  Note that `va_error_t*`
   arguments never print, and never consume a `~` format, but
   always just return the stream error.

 - `m` prints the (error) status of the referenced item.  This is
   for custom printers, and it is encoded as `VA_MODE_STAT`.
   The pre-defined data types have no error values and thus no
   error is printed.  Retrieving the stream error using `va_error_t`
   is another topic, as it does not print anything.
   The letter `m` is inspired by the GNU extension `%m`, which prints
   `strerror(errno)`, which this library does not support natively
   (to avoid depending on `<errno.h>`).

 - `~` prints `~` characters.  By default, one is printed.  The
   width gives the number of tildes, e.g. `~5~` prints `~~~~~`,
   and `~0~` prints nothing.  `~*~` reads the width from an
   argument.  The use of precision and justification flags is
   reserved for future use, and it is unspecified how the library
   handles them.

 - any letter mentioned above in lowercase only also exists in
   uppercase, and then prints whatever is usually printed in lowercase
   in uppercase, like like hexadecimal digits or numeric base prefixes
   like `0B` or `0X`.

 - any format character not mentioned above is reserved for future
   use.  If used, the argument is skipped, and the `VA_E_FORMAT` error
   is set in the stream.

 - any combination of format character and type not mentioned above
   prints in default notation.

## Parameter Types

The following function parameter types are recognised.  Note that
enums are not listed here, because of the weak type system of C, where
enum constants have type 'int' and enum types match type 'int' in
_Generic.

 - `int`, `unsigned`, `char`, `signed char`, `unsigned char`, `short`,
   `unsigned short`, `long`, `unsigned long`, `long long`,
   `unsigned long long`: these are integer and are printed
   in unsigned or signed decimal integer notation by default.

   This means that `char`, `char16_t`, and `char32_t` all print in
   numeric format by default, not in character format, as they are not
   distinct types.  For interpreting them as a 1-element Unicode
   codepoint string, `c` format should be used.

   Also note that character constants like `'a'` have type `int` in C
   and print numerically by default.

 - `_Bool` (or `bool` with `<stdbool.h>`): prints a boolean type.
   This is the only enum in C that does not match an `int` type in
   `_Generic`, so it is supported.  Note that `true` and `false` still
   have type `int` and not `bool`, so only variables of type `bool`
   will print in boolean mode.  This prints `true` or `false` by
   default or `0` and `1` if a numeric format is used: `d`, `u`,
   `x`, `o`, `b`.

 - `char *`, `char const *`: 8-bit character strings or
   arrays.  They print as is by default.

   The default string encoding is UTF-8,  It can be reset to
   a user encoding by #defining `va_char_p_decode`.  Also see
   the section on encoding below.

   Unquoted, `NULL` prints empty and sets the `VA_E_NULL` error.
   Also see the section on quotation below.

   If the input decoder encounters an incomplete UTF-8 sequence right
   in front of the terminating `NUL` character, it will return the
   bytes of the incomplete sequence as decoding errors.  However, if a
   precision, i.e., maximum string size is specified, it can be made
   to stop decoding before the incomplete sequence without a decoding
   error.  This way, strings can be printed in chunks without errors.
   To trigger this behaviour, a pointer to a string or an iterator can
   be used, so the final string position, i.e., the first byte of the
   incomplete sequence at the end, can be queried in order to start a
   new string chunk with the incomplete sequence at the beginning,
   followed by, hopefully, the missing bytes of the UTF-8 sequence
   from the next chunk.

 - `char16_t *`, `char16_t const *`: 16-bit character strings or
   arrays.  The default encoding is UTF-16, which can be
   switched using `va_char16_p_decode`.

   Unquoted, `NULL` prints empty and sets the `VA_E_NULL` error.

   If the input decoder encounters an high UTF-16 surrogate right in front
   of the terminating `NUL` character, it will return the high surrogate
   as a decoding error.  However, if a precision, i.e., maximum string
   size is specified, it will stop decoding before the high surrogate.
   This can be used for chunked printing like with UTF-8.

 - `char32_t *`, `char32_t const *`: 32-bit character strings or
   arrays.  The default encoding is UTF-32, which can be
   switched using `va_char32_p_decode`.

   Unquoted, `NULL` prints empty and sets the `VA_E_NULL` error.

 - `Char **`, `Char const **`: pointers to pointers to
   characters, i.e., pointers to string, will print the string
   and then update the pointer to point to the code
   unit just behind the last one that was read from the
   string.  With no precision given in the format, they will
   point to the terminating NUL character.  When these
   parameters are printed multiple times using the `=` flag,
   the string will be reset each time and the updated value
   will correspond to the end position during the last print
   of the string.

 - `va_error_t*`: this retrieves the error code from the
   stream and writes it into the passed struct.  This can
   be used to check for encoding or decoding errors, out
   of memory conditions, or hitting the end of the output
   array.  `NULL` must not be passed as a pointer.

 - `va_read_iter_t*`: this is an internal type to read from
   strings.  There are quite a few constraints on how to
   define a proper `va_read_iter_t`, which are not all
   documented here.

 - `va_span_t*`: this is a length delimited string for printing
   non-NUL terminated strings or prefixes of strings.  It is
   an alternative way to specify the string size in the argument
   directly instead of using the precision in the format specifier.
   When strings are specified this way, embedded U+0000 (NUL)
   characters are passed down, so quotation may print them as
   \u0000 or \000 or similar.  NUL characters are never printed
   verbatim into the output stream, however, because the output
   stream is assumed to be text.

 - `va_span16_t*`: the same as `va_span_t`, but for `char16_t` strings.

 - `va_span32_t*`: the same as `va_span_t`, but for `char32_t` strings.

 - `va_print_t*`: user-defined printer for a value of an arbitrary
   type (there is a separate chapter on this, below).

 - anything else: is tried to be converted to a pointer and
   printed in hexadecimal encoding by default, i.e., in `~x`
   format.

## Library Modules

### Printing Into Fixed Size Arrays

```c
#include <va_print/char.h>
```

To print into an string of characters up to a given number of elements
in the array, the following function can be used, and it returns the
pointer to the string.  The resulting string is always NUL terminated,
i.e., the maximum string length is one less than the passed element
count.

```c
char s[20];
char *t = va_snprintf(s, sizeof(s), "foo~s", 5);
assert(s == t);
```

The target buffer's type may be 8, 16, or 32 bit characters -- no need
to use a different function.  For anything but `char` buffers, use
`va_countof()` for the array size, so that the number of elements, not
the number of bytes, is used as the string size.


```c
char16_t s1[20];
char16_t *t1 = va_snprintf(s1, va_countof(s1), "foo~s", 5);

char32_t s2[20];
char32_t *t2 = va_snprintf(s2, va_countof(s2), "foo~s", 5);
```

The `va_countof()` values is inferred when using `va_sprintf`. This
is different from the standard C `sprintf` function, which unsafely
assumes a sufficiently large string -- you cannot express that with
this library, but you'd have to use `snprintf` with a large size
instead.

```c
char s[20];
char *t = va_sprintf(s, "foo~s", 5);
```

It is possible to print into a compound literal of a given
size and return the pointer to that string.  In this case,
no character array can be used to infer the string type,
so it is encoded in the function name.  There are functions
for `char`, `char16_t`, and `char32_t` strings, as well as
a generic version that takes as first argument the string
character type.
```c
char     *t1a = va_nprintf (20, "foo~s", 5);
char16_t *t2a = va_unprintf(20, "foo~s", 5);
char32_t *t3a = va_Unprintf(20, "foo~s", 5);
char     *t1b = va_gnprintf(char, 20, "foo~s", 5);
char16_t *t2b = va_gnprintf(char16_t, 20, "foo~s", 5);
char32_t *t3b = va_gnprintf(char32_t, 20, "foo~s", 5);
```

The stream for printing can also be generated separately and then used
for iterative printing using `va_iprintf`.  The stream makes sure not
to print past the end of the char array.

```c
char buff[20];
va_stream_char_p_t stream = VA_STREAM_CHAR_P(buff, va_countof(buff));
va_iprintf(&stream, "foo");
va_iprintf(&stream, "bar ~u", 55);
va_iprintf(&stream, "longer than the string, will be cropped");
...
```

There is a `stream.pos` counter for the current write index in the
array, i.e., the string length of the encoded byte sequence.  `pos`
increments up to `stream.size-1`, but no further (note that `size==0`
is an illegal configuration, because then there is no space for NUL
terminating the string).

Creating a `va_stream_char_p_t` with the buffer equal to `NULL` is
explicitly allowed.  Putting the bytes into a char array will then be
inhibited.  The printer still increments `stream.pos` up to
`stream.size-1`, so by this, `strnlen()` functionality can be
implemented on the resulting string.  This is exactly how `va_zprintf`
(mnemonic: `siZe`) works.

To determine whether the stream was truncated, i.e., whether the
buffer was too small for the print result, the stream's error code can
be checked for the `VA_E_TRUNC` error code value after printing is
done.

```c
...
va_error_t e;
va_iprintf(&stream, "", &e);
if (e.code != VA_E_OK) {
    /* ... some stream error occurred ... */
}
...
```

Alternatively, if you have a stream anyway, there is
`va_stream_get_error()` that returns the stream's error code.
There is also `va_strerror()` to get the enum value as a string.

```c
...
unsigned ec = va_stream_get_error(&stream);
if (ec != VA_E_OK) {
    va_fprintf(stderr, "ERROR: found ec=~s\n", va_strerror(ec));
    /* ... more error handling */
}
```

Note that there is no `%n` equivalent format specifier for reading the
printed length; use `stream.pos` instead.  Or use `va_zprintf` to
compute the needed array size.

There is also `va_lprintf` to count the length of the string, i.e.,
the number of codepoints written, instead of the number of encoded
bytes.

### Printing Into Growing Vectors

```c
#include <va_print/alloc.h>
```

It is possible to print into a string that is allocated grows using
`malloc()`:

```c
char *c = va_asprintf("foo~s", msg);
...
free(c);
```

Here, the `va_alloc()` function is implicitly used to allocate,
possibly reallocate while printing, and possibly freeing the string in
case of an out-of-memory error.

For `char16_t*` and `char32_t*` target strings, there are
`va_uasprintf` and `va_Uasprintf`, resp.

`va_alloc` is a wrapper around `realloc` and `free`.  Any compatible
function with the same prototype can be used instead.

A user defined allocation function can be supplied by using the
`va_axprintf` function, which is just like `va_asprintf`, but takes
the allocator function of type `void *(void *, size_t nmemb, size_t
size)` as parameter, which is used for allocation, reallocation, and
freeing (with `nmemb==0`).  For `char` strings, the function is
invoked with `size==1`.

```c
char *c = va_axprintf(va_alloc, "foo~s", msg);
...
free(c);
```

For `char16_t` and `char32_t` output strings, there is `va_uaxprintf`
and `va_Uaxprintf`, resp.  The allocator function will then be invoked
with a `size==2` for `char16_t` and `size==4` for `char32_t`.

It is also possible to create a stream for iterative printing.

```c
va_stream_vec_t stream = VA_STREAM_VEC(va_alloc);
va_iprintf(&stream, "foo");
va_iprintf(&stream, "bar ~u", 55);
```

For 16 and 32 bit chars, there is `va_stream_vec16_t` plus
`VA_STREAM_VEC16` and `va_stream_vec32_t` plus `VA_STREAM_VEC32`.

### Printing Into Files

```c
#include <va_print/file.h>
```

To print into `FILE*` files, there is `va_fprintf`, which returns
nothing.

```c
va_fprintf(stderr, "foo~s", msg);
```

There is also `va_printf` that prints into `stdout` and `va_eprintf`
that prints into `stderr`.  They also both return nothing.

```c
va_printf("foo~s", msg);
va_eprintf("foo~s", msg);
```

To write `char16_t` or `char32_t` streams into files, the encodings
`UTF-16BE` and `UTF-32BE` are used by default.  Functions for this
are called `va_ufprintf` and `va_Ufprintf`, resp.

For files, there is a stream type `va_stream_file_t` that can be
constructed using `VA_STREAM_FILE`, e.g., to iteratively print.

```c
va_stream_file_t stream = VA_STREAM_FILE(stderr);
va_iprintf(&stream, "foo");
va_iprintf(&stream, "bar ~u", 55);
va_iprintf(&stream, "longer than the string, will be cropped");
...
```

The 16-bit and 32-bit versions use the same stream type, and the
constructors are called `VA_STREAM_FILE16` and `VA_STREAM_FILE32`,
resp.  `


### Printing Into Raw File Descriptors

```c
#include <va_print/fd.h>
```

To print into `int` typed file descriptors, there is `va_dprintf`, which
returns nothing.

```c
va_dprintf(2, "foo~s", msg);
```

To write `char16_t` or `char32_t` streams into files, the encodings
`UTF-16BE` and `UTF-32BE` are used by default.  Functions for this
are called `va_udprintf` and `va_Udprintf`, resp.

For file descriptors, there is a stream type `va_stream_fd_t` that
can be constructed using `VA_STREAM_FD`, e.g., to iteratively print.

```c
va_stream_fd_t stream = VA_STREAM_FD(2);
va_iprintf(&stream, "foo");
va_iprintf(&stream, "bar ~u", 55);
va_iprintf(&stream, "longer than the string, will be cropped");
...
```

The 16-bit and 32-bit versions use the same stream type, and the
constructors are called `VA_STREAM_FD16` and `VA_STREAM_FD32`, resp.
`

### Printing non-NUL Terminated Strings

One way to print non-NUL terminated strings or prefixes of strings
is by specifying the 'precision' in the format.  It is the length
in `char`, `char16_t`, or `char32_t` elements and not the number of
extracted codepoints, exactly for this purpose.

```c
char const *data = "abcdef";
size_t size = 3;
va_fprintf(stderr, "token=~.*qs", size, data);
```
This prints `token="abc"`.

An alternative way of controlling this is to pass a pointer to
`va_span_t` to the printer, which contains the data and size:

```c
char const *data = "abcdef";
size_t size = 3;
va_fprintf(stderr, "token=~qs", (&(va_span_t){ size, data }));
```

This also prints `token="abc"`.

Note that strings specified by their size may contain U+0000 (NUL)
characters, and they are quoted accordingly, if requested:

```c
va_fprintf(stderr, "token=~qs", (&(va_span_t){ 1, "" }));
```

This also prints `token="\000"`.

There are similar types `va_span16_t` and `va_span32_t` for wide
character strings.

### Computing String Lengths

```c
#include <va_print/len.h>
```

The function `va_lprintf` returns the number of codepoints printed
into an output stream.  This is the string length regardless of output
encoding.

```c
size_t cp_count = va_lprintf("foo~s", msg);
```

This is not a good function for computing array sizes -- use the
`va_zprint` family instead.


### Computing String Array Sizes

```c
#include <va_print/char.h>
```

To compute the size of the array needed to store a given printed
string, there is `va_zprintf`.

```c
size_t n = va_zprintf("foo~s", msg);
char *s = malloc(n);
va_error_t e;
va_snprintf(s, n, "foo~s", msg, &e);
assert(e.code == VA_E_OK);
```

This function counts the encoded size of the needed array, i.e., it
also includes the `NUL` character in the count, and it counts for each
codepoint, how many UTF-8 (or whatever encoding is used) bytes are
used for each codepoint.  This function is, therefore, useful for
computing array sizes that fit the printed string exactly.

For `char16_t` and `char32_t` based strings, the function is called
`va_uzprintf` and `va_Uzprintf`, resp.

There is a generic version that can be passed the array element
type as the first parameter.

```c
typedef SomeCharacterType MyChar;
...
size_t n = va_gzprintf(MyChar, "foo~s", msg);
MyChar *s = malloc(sizeof(MyChar) * n);
va_snprintf(s, n, "foo~s", msg);
```

## Unicode

Internally, this library uses 32-bit codepoints with 24-bit payload
and 8-bit tags for processing strings, and by default, the payload
representation is Unicode.  The library tries not to interpret the
payload data unless necessary, so that other encodings could in
principle be used and passed through the library.

The only place the core library uses Unicode interpretation is when
quoting C or JSON strings for codepoints >0x80 (e.g., when formatting
with `~0qs`), and if a decoding error is encountered or if the value
is not valid Unicode, then it uses \ufffd to show this, because the
quotation using \u or \U would otherwise be a lie.

The internal representation allows any value within 24 bits to be used
for codepoints.  0 is interpreted as 'end of string' and is never
printed into the output stream.

UTF-8, -16, and -32 encoders and decoders check that the Unicode
constraints are met, like excluding anything above 0x10FFFF and high
and low UTF-16 surrogates, and detecting decoding errors according to
the Unicode recommendations and best practices.  The encoder/decoder
pairs usually try to pass through faulty sequences as is, if possible,
e.g., reading ISO-8859-1 data from an UTF-8 `~s` and printing it into
an UTF-8 output stream preserves the original ISO-8859-1 byte
sequence, although the intermediate steps do raise 'illegal sequence'
errors.

Integers print without Unicode checks, i.e., if an integer is printed
as a character using `~c`, then the lower 24 bits is passed down to
the output stream encoder as is.  If integers larger than 0xffffff are
tried to be printed with `~c`, this results in a decoding error, and
only the lower 24 bits are used.

## Encodings

The library supports different string encodings for the format string,
for input strings, and for output streams.  The defaults are UTF-8,
UTF-16, or UTF-32.  This can be switched by setting the following
#defines before including headers of this library, i.e., it cannot be
switched dynamically out of the box, because this would mean that all
the encoding modules would always be linked.  Dynamic switching can
be added by defining a new encoding that internally switches dynamically.

The following #defines switch function names:

### Format String Encoding

The default is UTF-8, -16, or -32 encoding, and it can be changed
by #defining before `#include <va_print/...>`:

    #define va_char_p_format utf8
    #define va_char16_p_format utf16
    #define va_char32_p_format utf32

These macros are appended to an identifier to find the appropriate
reader for the format string as follows:

    va_char_p_read_vtab ## va_char_p_format
    va_char16_p_read_vtab ## va_char16_p_format
    va_char32_p_read_vtab ## va_char32_p_format

When using a different encoding than the default, it must be ensured
that the corresponding vtab declarations are visible.

### String Value Encoding

The default for reading string values is UTF-8, -16, or -32 encoding,
for `"..."`, `u"..."`,and `U"..."` strings,resp.  The default can be
changed by defining one of the following macros before `#include <va_print/...>`:

    #define va_char_p_decode utf8
    #define va_char16_p_decode utf16
    #define va_char32_p_decode utf32

These macros are appended to an identifier to find the appropriate
reader for the string value as follows:

    va_xprintf_char_p_ ## va_char_p_decode
    va_xprintf_char_pp_ ## va_char_p_decode
    va_xprintf_char_const_pp_ ## va_char_p_decode
    va_xprintf_char16_p_ ## va_char16_p_decode
    va_xprintf_char16_pp_ ## va_char16_p_decode
    va_xprintf_char16_const_pp_ ## va_char16_p_decode
    va_xprintf_char32_p_ ## va_char32_p_decode
    va_xprintf_char32_pp_ ## va_char32_p_decode
    va_xprintf_char32_const_pp_ ## va_char32_p_decode

Note that for each parameter type, a different printer function is used,
so for a different encoding, three functions need to be provided.  A
typical such function implementation looks as follows:

    va_stream_t *va_xprintf_char_p_utf8(
        va_stream_t *s,
        char const *x)
    {
        va_read_iter_t iter = VA_READ_ITER(&va_char_p_read_vtab_utf8, x);
        return va_xprintf_iter(s, &iter);
    }

### Output Stream Encoding

For encoding strings into character arrays, the default encoding is
UTF-8, UTF-16, or UTF-32, depending on the string type.  To override
the default, the following #defines can be set
before `#include <va_print/...>`.

    #define va_char_p_encode utf8
    #define va_char16_p_encode utf16
    #define va_char32_p_encode utf32

These are suffixed to find the vtab object for writing:

    va_char_p_vtab_ ## va_char_p_encode
    va_char16_p_vtab_ ## va_char16_p_encode
    va_char32_p_vtab_ ## va_char32_p_encode

For dynamically allocated arrays, there are separate #definitions:

    #define va_vec8_encode utf8
    #define va_vec16_encode utf16
    #define va_vec32_encode utf32

These are suffixed to find the vtab object for writing:

    va_vec_vtab_ ## va_vec_encode
    va_vec16_vtab_ ## va_vec16_encode
    va_vec32_vtab_ ## va_vec32_encode

For `FILE*` output, the default encoding is UTF-8, UTF-16BE,
and UTF-32BE, depending on output character width.  The
following #defines correspond to the encoding:

    #define va_file8_encode utf8
    #define va_file16_encode utf16be
    #define va_file32_encode utf32be

These are suffixed to find the vtab object for writing:

    va_file_vtab_ ## va_file_encode
    va_file16_vtab_ ## va_file16_encode
    va_file32_vtab_ ## va_file32_encode

For `int` file descriptor output, the default encoding is UTF-8,
UTF-16BE, and UTF-32BE, depending on output character width.  The
following #defines correspond to the encoding:

    #define va_fd8_encode utf8
    #define va_fd16_encode utf16be
    #define va_fd32_encode utf32be

These are suffixed to find the vtab object for writing:

    va_fd_vtab_ ## va_fd_encode
    va_fd16_vtab_ ## va_fd16_encode
    va_fd32_vtab_ ## va_fd32_encode

## Quotation

### C/C++ quotation

- `q` quotation option in format specifier
- when printing integers, this is ignored
- when printing pointers, this adds the `#` flag, i.e., the
  `0x` prefix is printed
- when printing strings, this selects C format quoted output
- `NULL` strings print as `NULL`, and do not set the
  `VA_E_NULL` error, in contrast to unquoted printing.
- without `#`, prints quotation marks, single for `c` and `C`,
  conversion, otherwise double.
- with `z` prints the string size indicator based on the input
  string: empty for `char`, `u` for `char16_t`, and `U` for
  `char32_t` (and also `U` for 64-bit ints).
- quotation of unprintable characters <U+0080 is done using
  octal quotation.
- quotation of some characters in special notation:
  `\t`, `\r`, `\n`, `\'`, `\"`, `\\`.
- `0` flag quotes all non-ASCII using `\u` or `\U`.  Note
  that `\x` is not used, because it may not terminate, so
  quoting `\x1` plus `1` is more complicated.
- with `0` flag, chars that are marked as decoding errors are
  quoted as `\ufffd`, the replacement character, to avoid
  printing encoding errors with `\u` quotation, which would
  make the resulting string more wrong than with only the
  encoding errors.  Without `0` flag, encoding errors are
  passed through if the input encoding equals output
  encoding, otherwise `U+FFFD` is encoded.
- upper case formats use upper case letters in hexadecimals

Examples:

- `va_printf("~qs", "foo'bar")` prints `"foo\'bar"`.
- `va_printf("~qs", "foo'bar")` prints `"foo\'bar"`.
- `va_printf("~qzs", u"foo'bar")` prints `u"foo\'bar"`.
- `va_printf("~qc", 10)` prints `'\n'`.
- `va_printf("~qzc", 10)` prints `U'\n'`
- `va_printf("~#qc", 16)` prints `\020`.
- `va_printf("~#0qc", 0x201c)` prints `\u201c`.
- `va_printf("~#0qC", 0x201c)` prints `\u201C`.
- `va_printf("~qa", (void*)18)` prints `0x12` (on normal machines)
- `va_printf("~qa", 18)` prints `18`
- `va_printf("~0qa", u"\xd801")` prints `"\xfffd"`

### Java/JSON quotation

- `Q` quotation option in format specifier
- Like C, but always uses `\u` or `\U` and never octal
- `NULL` strings print as `null`, and do not set the
  `VA_E_NULL` error, in contrast to unquoted printing.
- the `z` flag is ignored (no `u` or `U` prefixes are printed).

Examples:

- `va_printf("~Qs", "foo'bar")` prints `"foo\'bar"`.
- `va_printf("~Qc", 10)` prints `'\n'`.
- `va_printf("~#Qc", 16)` prints `\u0010`.
- `va_printf("~#0Qc", 0x201c)` prints `\u201c`.
- `va_printf("~#0QC", 0x201c)` prints `\u201C`.
- `va_printf("~Qa", (void*)18)` prints `0x12` (on normal machines)
- `va_printf("~Qa", 18)` prints `18`

### Bourne Shell quotation

- `k` quotation option in format specifier (mnemonic: Korn
  Shell quotation)
- when printing integers, this is ignored
- when printing pointers, this adds the `#` flag, i.e.,
  the `0x` prefix is printed
- when printing strings, this selects Shell quoted format
- `NULL` strings print as empty string, and set the
  `VA_E_NULL` error, just like unquoted printing.
- the empty string is printed as `''`
- uses single quotes if necessary
- without `#`, prints quotation marks if necessary
- others print no quotation marks for in-string printing
- this actually quotes nothing except the single quotation
  mark.
- chars marked as decoding errors are not quoted, but passed
  through.

Examples:

- `va_printf("~ks", "ab")` prints `ab`.
- `va_printf("~ks", "")` prints `''`.
- `va_printf("~#ks", "")` prints ``.
- `va_printf("~ks", "a b")` prints `'a b'`.
- `va_printf("~ks", "a'b")` prints `'a'\''b'`.
- `va_printf("~#ks", "a'b")` prints `a'\''b`.
- `va_printf("~ka", (void*)18)` prints `0x12` (on normal machines)
- `va_printf("~ka", 18)` prints `18`

### User-Defined Quotation

The quotation mechanism of the library can be extended by own
quotation techniques.  The API for this is currently preliminary
and may change.

For implementing custom quotation, the `impl.h` header file needs to
be included to get access to the internal programming API:

```c
#include <va_print/impl.h>
```

There is a definition of a struct `va_quotation_t` which has three
entries to be defined for a quotation mechanism:

- `unsigned delim[2]`: the delimiter with which to quote. array index [0]
  is used for characters and index [1] is used for string
  quotation.  The `VA_DELIM(prefix, frontquote, backquote)` macro constructs
  an entry.  The quotation characters `frontquote` and `backquot` must be
  in the BMP, i.e., smaller than or equal to `U+FFFF`.  The `prefix` character
  must be smaller than or equal to U+00FF.  If it is 0xff, then the
  prefix is selected if the `z` modifier is used based on the string character
  type: empty for `char`, `u` for `char16_t` and `U` for `char32_t`.

- `bool (*check_quote)(va_stream_t *s, unsigned c)`: if NULL, quotation is
  always used. If non-NULL, this function is used on each character of the
  string to check whether quotation is needed.  If the function returns
  non-false for any of the characters, then quotation is needed.

- `bool (*check_flush)(va_stream_t *s)`: if non-NULL, will be invoked at the
  end of the string quotation check (only if check_quote is non-NULL) to
  check again whether quotation is needed.  `check_quote` and `check_flush`
  may use `s->qctxt` for storing some state, e.g., for detecting and empty
  string (which needs quotation in Shell quotation).

- `void (*render_quote)(va_stream_t *s, unsigned ch)`: for the actual
  quotation of a character.  This must invoke one of the `va_stream_render*()`
  functions for writing the quoted representation of the character into
  the output stream.  The renderer can store context in `s->qctxt`, an
  `unsigned`, during quotation rendering.  It is initialised to `0`
  when rendering starts.

- `void (*render_flush)(va_stream_t *s)`: callback for the quotation to
  signal the end of the quoted string.  Maybe NULL if not needed.

There are the following rendering functions for the `render_quote` method:

- `va_stream_render(s,c)`: prints the character verbatim into the output
  stream.  Note that this cannot be done manually by a different function,
  because this function also contains the logic for counting string
  widths, etc.
- `va_stream_render_quote_u(s,c)`: prints as `\u0123` or `\U01234567` in
  hexadecimal notation
- `va_stream_render_quote_oct(s,c)`: prints as `\012` in octal notation.

For setting a quotation technique, a `va_quotation_t` needs to be
initialised and set using `va_quotation_set()`.

```c
static va_quotation_t const my_quotation = {
    .delim = { VA_DELIM(0, '<', '>'), VA_DELIM(0, '|', '|') },
    .render_quote = my_render_quote,
};
void my_init(void)
{
    va_quotation_t const *old = va_quotation_set(VA_QUOTE_qq, &my_quotation);
}
```

This sets the `qq` prefix to use `my_quotation` as a quotation method.
There are currently 8 different quotation method slots:

- `0`: the default if no `q`, `Q`, `k`, or `K` modifier is specified
- `VA_QUOTE_q`: used if the modifier `q` is given
- `VA_QUOTE_Q`: used if the modifier `Q` is given
- `VA_QUOTE_k`: used if the modifier `k` is given
- `VA_QUOTE_K`: used if the modifier `K` is given
- `VA_QUOTE_qq`: used if the modifier `qq` is given
- `VA_QUOTE_QQ`: used if the modifier `QQ` is given
- `VA_QUOTE_kk`: used if the modifier `kk` is given

The prefixes `q`, `k`, and `Q` are predefined as described in the
previous sections.  The others do not quote by default and are free
for adding user quotation methods.  Note that is possible to set
quotation 0 so that it is used when no quotation modifier is given.

## User Defined Printers

For user types, printers can be defined so that you do not need to
print into an intermediate string buffer, but you can directly print
into the output stream.  This saves space for the temporary string
buffer and avoids thinking about buffer sizes.

The library provides the type `va_print_t*` that can be passed as an
argument to the printing functions instead of the value itself.  This
provides the library with a user-defined callback for printing, and
also encapsulates the value to be printed.

`va_print_t` has a callback function `void (*print)(va_stream_t *,
va_print_t*)` that the user can fill in to print the user value.  The
function can make use of `va_iprintf` to stringify the value.  The
original stream's format is passed via `width`, `prec`, and `opt`
value in the `va_print_t` struct so that the printer can query them.

To define a printer for a custom type, a derived struct of
`va_print_t` can be used to encapsulate the value, or the provided
`value` can be used to store store a pointer you your value into
`va_print_t::value` if that is sufficient information.  E.g., for a
simple pair of integers:

```C
typedef struct {
    unsigned a,b;
} my_pair_t;
```

a printing function is defined for values of this user type:

```C
static void my_print_pair(va_stream_t *s, va_print_t *p)
{
    my_pair_t const *v = p->value;
    va_iprintf(s, "(.a=~s .b=~s)", v->a, v->b);
}
```

When this is in place, to make the invocation easy, it is a good idea
to encapsulate the creation of a temporary `va_print_t` into a macro.
The following macro uses an `({...})` block to implement a type check,
because `va_print_t::value` is a void pointer.

```C
#define P_PAIR(_v) (&VA_PRINT(&my_print_pair,({my_pair_t const *v_=(v); v_;}))
```

With these definitions, values of the user type can be printed:

```C
my_pair_t pair = { 1, 2 };
va_fprintf(stderr, "pair=~s\n", P_PAIR(&pair));
```

This prints `pair=(.a=1 .b=2)` with the above definitions.

The custom printer framework handles width and quotation just like with
normal string types.

```C
va_fprintf(stderr, "quote(pair)=~-10qs\n", P_PAIR(&pair));
```

In contrast to the width, alignment, and quotation, The print
precision is not handled by the framework -- the print function needs
to handle it, because the semantics of a precision depends on the
type.  Also, `NULL` values, are not handled specially, but such values
are printed via the normal mechanism -- the framework does not examine
the `va_print_t::value` at all.

Different print formats must be handled by the user print function,
e.g., for printing the type or a pointer values -- nothing of this is
done by the framework.  E.g., with `VA_BGET(p->opt, VA_OPT_MODE)`, the
mode can be queries and one of the `VA_MODE_*` constants can be
checked.  See `va_print/impl.h` for the implementation API to access
the format options.

In the `P_PAIR` macro above, one could apply some `_Generic` magic to
select the right printer for a given object type, maybe even to
improve on the `({...})` type checking.

## Extensions

- This is type-safe, i.e., printing an int using "~s" will not
  crash, but just print the integer.

- `~b` and `~B` print binary, with optional `0b` or `0B` prefix.

- `~e` and `~E` print integers in Base32, with optional `0e` or `0E`
  prefix.  This could be handy for writing error codes: 0EINVAL,
  0EAGAIN, 0EIO, ...

- any meaningless format specifier (=letter) defaults to 'print in
  natural default form'.  It is recommended to use `~s` for default
  format printing of anything.

- The `=` modifier prints the last value again, possibly with a
  different format.  Note that the format containing `=` should not
  contain any `*`, because then the width/precision will be
  printed, not the last value, which is probably not what you want.

- The `q`, `Q`, `k`, and `K` modifiers mark different kinds of quotation.
  `q` is for C, `Q` is for Java/JSON, and `k` for Bourne/Korn Shells.

- The `t` format prints the input value type in C syntax.

- The `m` format is a custom format to print the status of an object,
  usually for the error status.

## Differences

- The `~x` specifier also prints negative signed numbers, again, due
  to type-safety.  Reinterpreting them as unsigned can be done with
  the `z` flag.

- The format specifiers are not needed to prevent the program from
  crashing, because the information about the type that is passed is
  not needed.  The format really only specifies 'print like ...', so
  by default it is recommended to just print with `~s`.

- Due to the type-safety, most length modifiers are not supported nor
  needed.  See `h`, `hh`, and `z` modifiers.

- for strings, the precision counts the number of output bytes in the
  standard, but in this library, it is the number of input elements in
  the array, i.e., the precision specifies the array size the string
  points to.  It is felt that input count is more useful, because
  it allows to print non-NUL terminated strings, while the output
  width can be controlled by a delimited printer.  Also, different
  glyph widths in Unicode means that the visual width cannot really
  be controlled by the output count, either.

- for strings, the width counts the number of characters that are
  printed, before encoding them in the output encoding.  This
  includes all characters needed for quotation.

- This library assumes that text is printed, not binary, so it will
  not output plain `\0`.

## Restrictions

- `%n` is not implemented, because pointers to integers are already
  used for strings, and the ambiguity between `size_t*` and
  `char32_t*` is common on many 32-bit systems, where both are
  `unsigned*` in C.  Distinguishing whether to read or to write based
  on the format string alone is also the opposite of what this library
  tries to do, and accidentally writing the print size into an
  `char32_t*` string is a weird bug I'd rather not make possible.

- `m$` syntax for reordering format strings is not supported, because
  it would require storing the parameters in an array and would
  counteract all the magic of the recursive expressions.  This would
  make the code much more complex and stack usage infeasible.  In
  fact, it would probably make the whole point of this library
  infeasible.  There is the extended `=` option for at least printing
  the same value multiple times, so `~d ~=#x` prints the same value
  decimal and hexadecimal, and `~qs ~=p` prints a string in C quotation
  and its pointer value.

- no floats, because support would be too large for a small library.
  Maybe it is added later -- it could be in a separate .o file that
  is only used if float arguments are actually used (the magic of
  _Generic: you would not pay for floats unless you use them).

- Of the size flags, `hh`, `h`, `l`, `ll`, `L`, `q`, `j`, `z`, `Z`,
  `t`, only `h`, `hh`, and `z` are implemented, with slightly
  different semantics to print unsigned integers: `h` applies a mask
  0xffff, `hh` applies a mask `0xff` and `z` reinterprets the given
  number as unsigned.  Due to the type-safety, the other flags are not
  needed as the library just prints whatever is thrown at it.

- `'...'` literals have type `int` in C, so values >0x7f, with its
  highest bit set, will be misinterpreted as illegal Unicode on
  compilers where `char` is signed.  On my compiler, printing
  `"~c",'\xfe'` prints a replacement characters, because `\xfe` equals
  `(int)0xfffffffe`, which is not valid Unicode, and this library
  has no chance to find out that this is in fact `(char)0xfe`.  So
  printing `'...'` literals is unfortunately broken, without a fix.
  Printing with `~hhc` works as expected (but `~zc` does not,
  because, `\xfe` is an `int`).  Printing `(char)'\xfe'` also works,
  but is more ugly in my opinion (I do not like casts much).

- Compiling `printf` with any modern compiler gives you compile time
  warnings about the argument type vs. format string consistency.  If
  these warnings are gone, there are usually no typing problems left
  for the target architecture (but compiling for other architectures
  may still have warnings and produce crashing code).

  For this library, no such warnings are be issued, because the
  argument passing is type safe and you cannot crash it with a wrong
  format specifier.  However, if the argument count and format
  specifier count do not match, then the output is likely wrong, and
  the function will have undesirable behaviour.  There is no compile
  time warning for this.

- gcc 6: The library itself uses relatively little stack.  But gcc
  (and also clang 3.8) accumulates the temporary stack objects in each
  function without reusing the stack space, i.e., each call to some
  print function builds up more stack at the call site. The temporary
  objects are clearly dead, but gcc keeps them. It does not help to add
  `({...})` or `do{...}while(0)` to formally restrict the official
  lifetime of the object to a block -- the compilers keep the object
  around.  This is highly undesirable here, but I have no idea how to
  prevent this.  -fconserve-stack and any other optimisations I tried
  don't change anything.

  gcc 11 fixes this (or maybe some earlier version), but it requires a
  block to limit the lifetime, even if the object is clearly dead.  I
  added `({...})` to the macros so that newer compilers produce much
  less stack usage at call sites.

## Q&A

- Q: Why formatted printing?

  A: Because it is nicer, and also it is feasible for Gnu gettext,
  which e.g. C++'s `cout<<` is not.  A: Because I like the string
  template based approach and find it more concise and can read it
  with less effort.

- Q: Is this perfect?

  A: Well, no.  It is hard to extend for other types to print.  The
  macro mechanisms used are near impossible to understand and causse
  weird error messages and wrong error positions (in my gcc).  The
  _Generic mechanism causes a ton of C code to be emitted for each
  print call -- the compiler throws almost all of it away, based on
  argument type, but looking at the pre-processed code is
  interesting. The number of arguments may be inconsistent with the
  format string without compile time warning.

- Q: I stack usage really low?

  A: Kind of, but not as much as I'd like.  It's around 250 bytes
  worst case on my x86-64.

- Q: What about code size?

  A: This generates more code at the call site, because each argument
  is translated to another function call.  Also, the temporary objects
  cause more stack to be used at the call site.  Interally, the library
  is OK wrt. code size, I think.

- Q: What about speed?

  A: Really?  This is about printing messages -- probably short ones
  (less than a few kB, I'd guess).  So while I did try not to mess it up,
  this is not optimised for speed.

- Q: Is this safer than `printf`?

  A: Definitely, I think.  There is absolutely no chance to give a
  wrong format specifier and access the stack (like `printf` does via
  `stdarg.h`) in undefined ways.  This is particularly true for
  multi-arch development where with `printf` you need to be careful
  about length specifiers, and you might not get a warning on your
  machine, but the next person will and it will crash there.  I
  usually need to compile a few times on multiple architectures to
  get the integer length right, e.g., `%u` vs `%lu` vs. `%llu`
  vs. `%zu`.

  This library's mechanism is also more convenient, because you do not
  need to think much about what you're printing with what format
  specifier, and there are no `PRId16` macros that obfuscate your
  portable code.  And you can use UTF-8, -16, -32 strings seamlessly
  and mix them freely.  You can print into a alloced or stack
  allocated compound literal safely, with error checking (end of
  string, out of memory, etc.) and guaranteed NUL termination.

- Q: Why do you use `~s` and not `%s`?

  A1: This did use `%s` at the beginning.  But the format strings must
  not be be confused with the standard C `printf`.  The format is not
  compatible with a `printf` call, and this is not a drop-in
  replacement.  E.g., in a larger code base, both this and old
  `printf` might be mixed, maybe during a transition period, or just
  because.  So programmers may see both styles.  With the different
  sigil, it is immediately clear which format is used in a print call
  when editing code, and confusion can hopefully be avoided.

## TODO

- ISO-8859-1 (because why not)

## Examples

Open a file with computed name, up to a fixed path length:

```c
#include <va_print/char.h>

FILE *open_text_rd(char const *dir, char const *file, unsigned suffix)
{
    return fopen(va_nprintf(80, "~s/~s~.s", dir, file, suffix), "rt");
}
```

The same with error checking about truncated string or en- or decoding
errors:

```c
FILE *open_text_rd(
    char const *dir, char const *file, unsigned suffix)
{
    va_error_t e;
    char *fn = va_nprintf(80, "~s/~s~.s", dir, file, suffix, &e);
    if (e.code != VA_E_OK) {
        return NULL;
    }
    return fopen(fn, "rt");
}
```

You can use 8-bit, 16-bit, or 32-bit characters seamlessly.  The
following uses UTF-16 as a parameter, but calls fopen() with an UTF-8
string.  The only change is the parameter type.  Just for fun, let's
use an UTF-32 format string:

```c
FILE *open_text_rd(
    char16_t const *dir, char16_t const *file, unsigned suffix)
{
    va_error_t e;
    char *fn = va_nprintf(80, U"~s/~s~.s", dir, file, suffix, &e);
    if (e.code != VA_E_OK) {
        return NULL;
    }
    return fopen(fn, "rt");
}
```

This can also be done by creating a dynamically allocated string.  The
`va_error_t` mechanism then protects against out-of-memory situation
(in which case the function deallocates what it allocated before and
returns NULL), and also against Unicode decoding/encoding errors, and
other traps.

```c
#include <va_print/alloc.h>

FILE *open_text_rd(char const *dir, char const *file, unsigned suffix)
{
    FILE *f = NULL;
    va_error_t e;
    char *fn = va_asprintf("~s/~s~.s", dir, file, suffix, &e);
    if (e.code == VA_E_OK) {
        f = fopen(fn, "rt");
    }
    free(fn);
    return f;
}
```

Using VLA, do the same with arbitrary length by pre-computing the length
using va_lprintf():

```c
#include <va_print/len.h>

FILE *open_text_rd(char const *dir, char const *file, unsigned suffix)
{
    char s[va_zprintf("~s/~s~.s", dir, file, suffix)];
    return fopen(va_sprintf(s, "~s/~s~.s", dir, file, suffix), "rt");

}
```

## How Does This Work?

The main idea is to use macro magic (both standard C99 and some extensions
from gcc, like allowing `__VA_ARGS__` to be empty etc.) to convert the
printf calls:

```c
x_printf(format);
x_printf(format, arg1);
x_printf(format, arg1, arg2);
...
```

Into a recursive call sequence:

```c
init(&STREAM(...), format);
render(init(&STREAM(...), format), arg1);
render(render(init(&STREAM(...), format), arg1), arg2);
...
```

The `STREAM()` is a temporary stream object, a compound literal, that
is used for state information when parsing the format string, and for
storing the output printer.  The pointer to this temporary object is
returned by all of the functions to the next layer of recursion.  The
`init()` initialises the format parser and the output stream (e.g. for
NUL termination and initial `alloc()`), and each `render()` consumes
one argument by printing it (once or more times) or using it as a
width or precision.

The macro magic is called `VA_REC()`.  Additional to what is described
above, it passes a first parameter to the `init()` and `render()`
calls to show whether the call is the last one of the expression. This
is done to be able to optimise the call site code generation, and to
allow sane error handling if the number of format specifiers and
arguments do not match.  You can try it with `gcc -E`:

```c
VA_REC(render, init, stream);
VA_REC(render, init, stream, a);
VA_REC(render, init, stream, a, b);
...
```

This becomes:

```c
init(0,stream);
render(0, init(1,stream), a);
render(0, render(1, init(1,stream), a), b);
```

The `init(0,...)` macro call is an extern function call that
initialises the stream, initialises the output stream (e.g., NUL
terminates a char array and/or allocs initial memory), and parses the
format string, so that even with no arguments, the expression behaves
in a sane way.

The `init(1,...)` macro call instead resolves to a fast inline
function that initialises the stream by just setting all the slots.
The format and output stream initialisation is then done by the first
`render(...)` invocation.  This way, there is the minimal number of
extern calls to keep the call site code small.

The `render()` resolves to a `_Generic()` call that selects the
appropriate printer based on the type of the argument, and based on
whether it's the last call of the expression, so that for each
argument, a different C functions may be invoked.  E.g.:

    int i;
    render(1,s,i)    -> print_int(s,i)
    render(0,s,i)    -> print_last_int(s,i)

    char const *x;
    render(1,s,x)    -> print_string(s,x)
    render(0,s,x)    -> print_last_string(s,x)

The `_last` variant finishes printing the format string even if no
more argument is given -- this is used to make error handling sane
and not just stop in the middle of the format string.
