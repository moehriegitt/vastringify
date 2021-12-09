# Type-Safe Printf For C11

This uses macro magic, compound literals, and _Generic to take
printf() to the next level: type-safe printing, printing into compound
literal char arrays, full support for UTF-8, 16, and 32, with good
error and pass-through handling.

The goal is to be safe and remove the need for varargs, and also to
concerve stack usage with the printfs that are provided, to make this
suitable even for embedded software.

The usual C printf formatting syntax is used, with some restrictions
and some extensions.

## Q&A

- Q: Why formatted printing?  A: Because it is nicer, and also it is
  feasible for Gnu gettext, which e.g. C++'s cout<< is not.  A:
  Because I like the string template based approach and find it
  more concise and can read it with less effort.

## TODO

- ISO-8859-1 (because why not)

## Examples

Open a file with computed name, up to a fixed path length:

    #include <va_print/char.h>

    FILE *open_text_rd(char const *dir, char const *file, unsigned suffix)
    {
        return fopen(va_nprintf(80, "%a/%a%.a", dir, file, suffix), "rt");
    }

The same with error checking about truncated string or en- or decoding
errors:

    FILE *open_text_rd(
        char const *dir, char const *file, unsigned suffix)
    {
        va_error_t e;
        char *fn = va_nprintf(80, "%a/%a%.a", dir, file, suffix, &e);
        if (e.code != VA_E_OK) {
            return NULL;
        }
        return fopen(fn, "rt");
    }

Using _Generic reduces the number of functions and macros, too, e.g.,
you can use 8-bit, 16-bit, or 32-bit characters seamlessly.  The
following uses UTF-16 as a parameter, but calls fopen() with an UTF-8
string.  The only change is the parameter type.  Just for fun, let's
use an UTF-32 format string:

    FILE *open_text_rd(
        char16_t const *dir, char16_t const *file, unsigned suffix)
    {
        va_error_t e;
        char *fn = va_nprintf(80, U"%a/%a%.a", dir, file, suffix, &e);
        if (e.code != VA_E_OK) {
            return NULL;
        }
        return fopen(fn, "rt");
    }

This can also be done by creating a dynamically allocated string with
realloc():

    #include <va_print/malloc.h>

    FILE *open_text_rd(char const *dir, char const *file, unsigned suffix)
    {
        char *fn = va_mprintf(realloc, "%a/%a%.a", dir, file, suffix);
        if (fn == NULL) {
            return NULL;
        }
        FILE *f = fopen(fn, "rt");
        free(fn);
        return f;
    }

Using VLA, do the same with arbitrary length by pre-computing the length
using va_lprintf():

    #include <va_print/len.h>

    FILE *open_text_rd(char const *dir, char const *file, unsigned suffix)
    {
        char n[va_lprintf("%a/%a%.a", dir, file, suffix)];
        return fopen(va_szprintf(n, "%a/%a%.a", dir, file, suffix), "rt");
    }

## Unicode

Internally, this library uses 32-bit codepoints with 24-bit payload
and 8-bit tags for processing strings, and by default, the payload
representation is Unicode.  The library tries not to interpret the
payload data unless necessary, so that other encodings could in
principle be used and passed through the library.

The only place the core library uses Unicode interpretation is when
quoting C or JSON strings for codepoints >0x80 (e.g., when formatting
with "%0qs"), and if a decoding error is encountered or if the value
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
e.g., reading ISO-8859-1 data from an UTF-8 "%s" and printing it into
an UTF-8 output stream preserves the original ISO-8859-1 byte
sequence, although the intermediate steps do raise 'illegal sequence'
errors.

Integers print without Unicode checks, i.e., if an integer is printed
as a character using "%c", then the lower 24 bits is passed down to
the output stream encoder as is.  If integers larger than 0xffffff are
tried to be printed with "%c", this results in a decoding error, and
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

    #define va_char_p_format utf8

    > va_va_take_... : char* format strings

### String Value Encoding

    #define va_char_p_decode utf8

    > va_xprintf_char_p_...        : char* strings
    > va_xprintf_char_pp_...       : char** strings
    > va_xprintf_char_const_pp_... : char const **strings

The 'pp' variants write back the end of the string, as the amount of code
units read from the string can be restricted using the format precision.

### Output Stream Encoding

    #define va_char_p_encode utf8

    > va_put_char_p_... : char* output stream

    #define va_file_encode   utf8

    > va_put_file_... : FILE* output stream

    #define va_malloc_encode utf8

    > va_put_vec_... : char* based vector output stream

## Quotation

### C/C++ quotation

- "q" modifier
- others print no quotation marks for in-string printing
- upper case formats use upper case letters in hexadecimals
- quotation of unprintable characters <U+0080 is done using
  octal quotation.
- without "#", prints quotation marks, single for "c" and "C",
  otherwise double.
- "0" quotes all non-ASCII using "\u" or "\U".  Note
  that "\x" is not used, because it may not terminate, so
  quoting "\x1" plus "1" is difficult.
- chars that are marked as decoding errors are quoted as
  "\ufffd", the replacement character, to avoid mixing encoding
  errors with "\u..." quotation, which would make the resulting
  string more wrong than with only the encoding errors.
- any kind of integer can be printed in char notation using
  "c" or "C".

### Java/JSON quotation

- "Q" modifier
- Like C, but always uses "\u" or "\U", but never octal

### Bourne Shell quotation

- "k" modifier
- uses single quote style
- without "#", prints quotation marks if necessary
- others print no quotation marks for in-string printing
- this actually quotes nothing except the single quotation
  mark.
- chars marked as decoding errors are not quoted, but passed
  through.

## Extensions

- This is type-safe, i.e., printing an int using "%s" will not
  crash, but just print the integer.

- "%b" and "%B" print binary, with optional "0b" or "0B" prefix.

- "%O" also prints octal, with "0" prefix

- "%e" and "%E" print integers in Base32, with optional "0e" or "0E" prefix.
  This could be handy for writing error codes: 0eINVAL, 0eAGAIN, 0eIO, ...

- any meaningless format specifier (=letter) defaults to 'print in
  natural default form'.  It is recommended to use "%a" for default
  format printing of anything.

- The "=" modifier prints the last value again, possibly with a
  different format.  Note that the format containing "=" should not
  contain any "*", because then the width/precision will be
  printed, not the last value, which is probably not what you want.

- The "q", "qq", and "Q" modifiers mark different kinds of quotation.
  "q" is for C, "Q" is for Java/JSON, and "k" for Bourne/Korn Shells.

## Differences

- This library assumes that text is printed, not binary, so it will never
  print '\0'.

- "%u" and "%i"/"%d" never reinterpret a sign bit: the mechanism is type-safe,
  so the actual value is printed.  "%u", "%d", and "%i" all just mean
  'print in decimal'.

- In general, the format specifiers carries much less importance,
  because the information about the type that is passed is not needed.
  The format really only specifies 'print like ...', so by default
  it is recommended to just print with "%a".

- Due to the type-safety, the size length modifiers (h, hh, the others
  are not needed) do not need to match the value that is passed, but
  the lengths 'h' and 'hh' are interpreted as masks 0xff and 0xffff,
  resp., i.e., you can print an unsigned long long's lowest bytes with
  "%hhd".

- for strings, the precision counts the number of output bytes in the
  standard, but in this library, it is the number of input elements in
  the array, i.e., the precision specifies the array size the string
  points to.

- for strings, the width counts the number of characters that are
  printed, before encoding them in the output encoding.  This
  includes all characters needed for quotation.

- If no format specifier is found, values are printed at the end of
  the format string in default notation (as if printed with %a).

- Note that '_' literals have type 'int' in C, so values >0x7f, with
  its highest bit set, will be misinterpreted as illegal Unicode on
  compilers that have signed chars.  On my compiler, printing
  "%c",'\xfe' prints a replacement characters, because 0xfffffe is not
  valid Unicode, and this libraries implementation has no chance to
  find out that this is in fact 0xfe (and not 0xfffe from a signed
  short promotion).  So printing '_' literals is unfortunately broken,
  without a fix.  Printing with "%hhc" works.  Printing (char)'\xfe'
  also works.

## Restrictions

- "%n" is not implemented, because pointers to integers are already
  used for strings, and the ambiguity between 'size_t*' and
  'char32_t*' is common on many 32-bit systems, where both are
  'unsigned*' in C.  Distinguishing whether to read or to write based
  on the format string alone is also the opposite of what this library
  tries to do, and accidentally writing the print size into an
  'char32_t*' string is a weird bug I'd rather not make possible.

- "m$" syntax for reordering format strings is not supported, because
  it would require storing the parameters in an array and would
  counteract all the magic of the recursive expressions.  This would
  make the code much more complex and stack usage infeasible.  In
  fact, it would probably make the whole point of this library
  infeasible.  There is the extended "=" option for at least printing
  the same value multiple times, so "%d %=#x" prints the same value
  decimal and hexadecimal, and "%qs %=p" prints a string in C quotation
  and its pointer value.

- no floats, because support would be too large for a small library.
  Maybe it is added later -- it could be in a separate .o file that
  is only used if float arguments are actually used (the magic of
  _Generic: you would not pay for floats unless you use them).

- Of the size flags, hh, h, l, ll, L, q, j, z, Z, t, only h and hh
  are implemented as masks 0xff and 0xffff, resp., because it was
  felt that the others do not make much sense in a type-safe setting
  as they exist mainly to ensure correct pointer arithmetics with
  stdargs.h based printf.

- gcc 6: The library itself uses relatively little stack.  But gcc
  (and also clang 3.8) accumulates the temporary stack objects in each
  function without reusing the stack space, i.e., each call to some
  print function builds up more stack at the call site. The temporary
  objects are clearly dead, but gcc keeps. It does not help to all
  ({...}) or do{...}while(0) to formally restrict the official
  lifetime of the object to a block -- the compilers keep the object
  around.  This is highly undesirable here, but I have no idea how to
  prevent this.  -fconserve-stack and any other optimisations I tried
  don't change anything.

  gcc 11 fixes this (or maybe some earlier version), but it requires a
  block to limit the lifetime, even if the object is clearly dead.  I
  added ({...}) to the macros so that newer compilers produce much less
  stack usage at call sites.
