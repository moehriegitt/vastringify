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
vs. `%zu`, even in portable code with different integer types.

'Type-safe' in this context does not mean that you get more compile
errors, but that the format specifier does not need to specify the
argument type, but just defines the print format.  In fact, format
strings with this library will have less compile-time checking (namely
none) than with modern compilers for standard `printf`.  This approach
is still safer.

## Compatibility

This library requires at least a C11 compiler (for `_Generic`,
`char16_t`, `char32_t`), and it uses a few gcc extensions that are
also understood by Clang and a few other compilers (`({...})`,
`,##__VA_ARGS__`, `__typeof__`, `__attribute__`).

## Synopsis

In the following, `Char` may be `char`, `char16_t`, or `char32_t`:

    #include <va_print/file.h>

    void
    va_fprintf(FILE *f, Char const *format, ...);

    void
    va_printf(Char const *format, ...);

    va_stream_file_t
    VA_STREAM_FILE(FILE *f);


    #include <va_print/char.h>

    Char *
    va_snprintf(Char *s, size_t n, Char const *format, ...);

    Char *
    va_szprintf(Char s[], Char const *format, ...);

    char *
    va_nprintf(size_t n, Char const *format, ...);

    char16_t *
    va_unprintf(size_t n, Char const *format, ...);

    char32_t *
    va_Unprintf(size_t n, Char const *format, ...);

    va_stream_charp_t
    VA_STREAM_CHARP(Char const *s, size_t n);


    #include <va_print/malloc.h>

    char *
    va_mprintf(void *(*realloc)(void *, size_t), Char const *, ...)

    char16_t *
    va_umprintf(void *(*realloc)(void *, size_t), Char const *, ...)

    char32_t *
    va_Umprintf(void *(*realloc)(void *, size_t), Char const *, ...)

    va_stream_vec_t
    VA_STREAM_VEC(void *(*realloc)(void *, size_t));

    va_stream_vec16_t
    VA_STREAM_VEC16(void *(*realloc)(void *, size_t));

    va_stream_vec32_t
    VA_STREAM_VEC32(void *(*realloc)(void *, size_t));


    #include <va_print/len.h>

    size_t
    va_lprintf(Char const *format, ...);

    va_stream_len_t
    VA_STREAM_LEN();


    #include <va_print/core.h>

    va_stream_...t *
    va_xprintf(va_stream_...t *s, Char const *format, ...)

    void
    va_iprintf(va_stream_...t *s, Char const *format, ...);

    void
    va_pprintf(va_stream_vtab_t *v, Char const *format, ...);


    #include <va_print/base.h>

    typedef struct { ... } va_stream_t;

    typedef struct { ... } va_stream_vtab_t;

    typedef struct { unsigned code; } va_error_t
    #define VA_E_OK
    #define VA_E_NULL
    #define VA_E_DECODE
    #define VA_E_ENCODE
    #define VA_E_TRUNC

    va_stream_t
    VA_STREAM(va_stream_vtab_t const *vtab)

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
information.  The format specifier "%a" can be used as a generic
'default' output format.

### Format Specifiers

Like in C, a format specifier begins with '%' followed by:

 - a list of flag characters
 - a width specifier
 - a precision specifier
 - a list of integer mask and quotation specifiers
 - a conversion letter

The following flags are recognised:

 - `#` print in alternative form.  For numeric format, a prefix to
   designate the base is prefixed to the value except to 0:
     - for `o` and base 8, `0` is prefixed
     - for `b` and base 2, `0b` is prefixed,
     - for `B` and base 2, `0B` is prefixed,
     - for `x` and base 16, `0x` is prefixed,
     - for `X` and base 16, `0X` is prefixed,
     - for `e` and base 32, `0d` is prefixed,
     - for `E` and base 32, `0E` is prefixed.

 For quoted strings, this inhibits printing of delimiting quotes.

 - `0` pads numerics with zero `0` on the left rather than
   with a space character ` `.  If a precision is given, this is
   ignored.

   For C and JSON quotation, this selects to quote non-US-ASCII
   characters using `\u` and `\U` instead of printing them in
   output encoding.

 - `-` selects to left flush instead of the default right flush.

 - ` ` (a space character U+0020) selects that a space is printed
   in front of positive signed integers.

 - `+` selects that a `+` is printed in front of positive signed
   integers.

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
encodings stored inside.  The input decoder will not read incomplete
encodings at the end of limited strings, but will stop before.  If
a pointer to a string pointer is passed, then the
pointer will be updated so that it points to the next character, i.e.,
the one after the last one that was read.

The following integer mask and quotation specifiers are recognised:

 - `h` applies the mask `0xffff` to an integer, then zero extends unsigned
   values, or sign extends signed values.
   E.g., `va_printf("%#hx",0xabcdef)` prints `-0x3211`.

 - `hh` applies the mask `0xff` to an integer, then zero extends unsigned
   values, or sign extends signed values.
   E.g., `va_printf("%hhX",0xabcdU)` prints `CD`.

 - `z` reinterprets a signed integer as unsigned (mnemonic: zero
   extension).  `z` is implicit in formats `u` (and `U`).
   E.g., `va_printf("%hhu", -1)` prints `255`.

 - `q` selects C quotation for strings and char format.  There is a
   separate section below to explain this.

 - `Q` selects JSON quotation for strings and char format.  There is a
   separate section below to explain this.

 - `k` selects Bourne or Korn shell quotation.  There is a
   separate section below to explain this.

Note that most of the usual length specifiers (`l`, `ll`, etc.) known
from C make no sense and are not recognised (nor ignored), because
type casting control in varargs is not needed here due to the
type-safety.

The following conversion letters are recognised:

 - `a` prints anything in default notation.

 - `o` selects octal integer notation for numeric printing (including
   pointers).

 - `d` or `i` selects decimal integer notation for numeric
   printing (including pointers).

 - `u` is equivalent to `zd`, i.e., prints a signed integer as
   unsigned in decimal notation.

 - `x` or `X` selects hexadecimal integer notation for numeric
   printing (including pointers).  `x` uses lower case digits,
   `X` upper case.  Note that this also prints signed numbers with
   a `-` if appropriate: `va_printf("%#x", -5)` prints `-0x5`.
   There's the `z` flag to print signed integers as unsigned.

 - `b` or `B` selects binary integer notation for numeric
   printing (including pointers).  `b` uses lower case prefix,
   `B` uses upper case.  The difference is only visible
   with the `#` flag.

 - `e` or `E` selects Base32 notation using the digits
   'a'..'z','2'..'7'.  `e` uses lower case digits and prefix,
   `E` uses upper case.

 - `p` prints like `#x`, and for any pointer, including strings,
   prints the pointer value instead of the contents.  Note that
   it also prints signed numbers: `va_print("%p",-5)` prints `-0x5`.

 - `P` is just like `p`, but with upper case hexadecial digits.

 - `c` prints integers (but not pointers) as characters, like a
   one-element string.  Note that the NUL character is not printed,
   but behaves like an empty string.  For string quotation where
   hexadecimals are printed, this uses lower case characters.

 - `C` is just like `c`, but in string quotation when hexadecimals
   are printed, uses upper case characters.

 - `t` prints the argument type before modifications, in C
   syntax: `int8_`..`int64_t`, `uint8_t`..`uint64_t`, `char*`,
   `char16_t*`, `char32_t*`, `void*`.  Note that `va_error_t*`
   arguments never print, and never consume a `%` format, but
   always just return the stream error.

 - any letter not mentioned above or any combination of letter and
   type not mentioned above prints in default notation.  If the letter
   is upper case, it uses upper case letters where appropriate.

Function parameters behind the last format specifier in the format
string are printed in default notation after everything that is
printed in the format string.

## Function Parameters

The following function parameter types are recognised:

 - `int`, `unsigned`, `char`, `signed char`, `unsigned char`, `short`,
   `unsigned short`, `long`, `unsigned long`, `long long`,
   `unsigned long long`: these are integer and are printed
   in unsigned or signed integer notation.
   This means that `char`, `char16_t`, and `char32_t` all print
   in numeric format by default, not in character format.  For
   interpreting them as a 1-element Unicode codepoint string,
   you need to use `c` format.  Also note that character
   constants like `'a'` have type `int` in C and print
   numerically by default.

 - `char *`, `char const *`: 8-bit character strings or
   arrays.  By default, the UTF-8 decoder is used to extract
   code points for printing.  Unquoted, `NULL` prints empty
   and sets the `VA_E_NULL` error.

 - `char16_t *`, `char16_t const *`: 16-bit character strings or
   arrays.  By default, the UTF-16 decoder is used to extract
   code points for printing.  Unquoted, `NULL` prints empty
   and sets the `VA_E_NULL` error.

 - `char32_t *`, `char32_t const *`: 32-bit character strings or
   arrays.  By default, the UTF-32 decoder is used to extract
   code points for printing.  Unquoted, `NULL` prints empty
   and sets the `VA_E_NULL` error.

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

 - anything else: is tried to be converted to a pointer and
   printed like a pointer, i.e., in `%#x` format.

## Unicode

Internally, this library uses 32-bit codepoints with 24-bit payload
and 8-bit tags for processing strings, and by default, the payload
representation is Unicode.  The library tries not to interpret the
payload data unless necessary, so that other encodings could in
principle be used and passed through the library.

The only place the core library uses Unicode interpretation is when
quoting C or JSON strings for codepoints >0x80 (e.g., when formatting
with `%0qs`), and if a decoding error is encountered or if the value
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
e.g., reading ISO-8859-1 data from an UTF-8 `%s` and printing it into
an UTF-8 output stream preserves the original ISO-8859-1 byte
sequence, although the intermediate steps do raise 'illegal sequence'
errors.

Integers print without Unicode checks, i.e., if an integer is printed
as a character using `%c`, then the lower 24 bits is passed down to
the output stream encoder as is.  If integers larger than 0xffffff are
tried to be printed with `%c`, this results in a decoding error, and
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
  `\t`, `\r`, `\n`, `\b`, `\f`, `\'`, `\"`, `\\`.
- `0` flag quotes all non-ASCII using `\u` or `\U`.  Note
  that `\x` is not used, because it may not terminate, so
  quoting `\x1` plus `1` is more complicated.
- chars that are marked as decoding errors are quoted as
  `\ufffd`, the replacement character, to avoid mixing encoding
  errors with `\u` quotation, which would make the resulting
  string more wrong than with only the encoding errors.
- upper case formats use upper case letters in hexadecimals

Examples:

- `va_printf("%qs", "foo'bar")` prints `"foo\'bar"`.
- `va_printf("%qzs", u"foo'bar")` prints `u"foo\'bar"`.
- `va_printf("%qc", 10)` prints `'\n'`.
- `va_printf("%qzc", 10)` prints `U'\n'`
- `va_printf("%#qc", 16)` prints `\020`.
- `va_printf("%#0qc", 0x201c)` prints `\u201c`.
- `va_printf("%#0qC", 0x201c)` prints `\u201C`.
- `va_printf("%qa", (void*)18)` prints `0x12` (on normal machines)
- `va_printf("%qa", 18)` prints `18`

### Java/JSON quotation

- `Q` quotation option in format specifier
- Like C, but always uses `\u` or `\U` and never octal
- `NULL` strings print as `null`, and do not set the
  `VA_E_NULL` error, in contrast to unquoted printing.
- the `z` flag is ignored.

Examples:

- `va_printf("%Qs", "foo'bar")` prints `"foo\'bar"`.
- `va_printf("%Qc", 10)` prints `'\n'`.
- `va_printf("%#Qc", 16)` prints `\u0010`.
- `va_printf("%#0Qc", 0x201c)` prints `\u201c`.
- `va_printf("%#0QC", 0x201c)` prints `\u201C`.
- `va_printf("%Qa", (void*)18)` prints `0x12` (on normal machines)
- `va_printf("%Qa", 18)` prints `18`

### Bourne Shell quotation

- `k` quotation option in format specifier (mnemonic: Korn
  Shell quotation)
- when printing integers, this is ignored
- when printing pointers, this adds the `#` flag, i.e.,
  the `0x` prefix is printed
- when printing strings, this selects Shell quoted format
- `NULL` strings print as empty string, and set the
  `VA_E_NULL` error, just like unquoted printing.
- uses single quotes if necessary
- without `#`, prints quotation marks if necessary
- others print no quotation marks for in-string printing
- this actually quotes nothing except the single quotation
  mark.
- chars marked as decoding errors are not quoted, but passed
  through.

Examples:

- `va_printf("%ks", "ab")` prints `ab`.
- `va_printf("%ks", "a b")` prints `'a b'`.
- `va_printf("%ks", "a'b")` prints `'a'\''b'`.
- `va_printf("%#ks", "a'b")` prints `a'\''b`.
- `va_printf("%ka", (void*)18)` prints `0x12` (on normal machines)
- `va_printf("%ka", 18)` prints `18`

## Extensions

- This is type-safe, i.e., printing an int using "%s" will not
  crash, but just print the integer.

- `%b` and `%B` print binary, with optional `0b` or `0B` prefix.

- `%e` and `%E` print integers in Base32, with optional `0e` or `0E`
  prefix.  This could be handy for writing error codes: 0eINVAL,
  0eAGAIN, 0eIO, ...

- any meaningless format specifier (=letter) defaults to 'print in
  natural default form'.  It is recommended to use `%a` for default
  format printing of anything.

- The `=` modifier prints the last value again, possibly with a
  different format.  Note that the format containing `=` should not
  contain any `*`, because then the width/precision will be
  printed, not the last value, which is probably not what you want.

- The `q`, `Q`, and `k` modifiers mark different kinds of quotation.
  `q` is for C, `Q` is for Java/JSON, and `k` for Bourne/Korn Shells.

## Differences

- This library assumes that text is printed, not binary, so it will never
  print '\0'.

- The `%x` specifier also prints negative signed numbers, again, due
  to type-safety.  Reinterpreting them as unsigned can be done with
  the `z` flag.

- The format specifiers are not needed to prevent the program from
  crashing, because the information about the type that is passed is
  not needed.  The format really only specifies 'print like ...', so
  by default it is recommended to just print with `%a`.

- Due to the type-safety, most length modifiers are not supported nor
  needed.  See `h`, `hh`, and `z` modifiers.

- for strings, the precision counts the number of output bytes in the
  standard, but in this library, it is the number of input elements in
  the array, i.e., the precision specifies the array size the string
  points to.

- for strings, the width counts the number of characters that are
  printed, before encoding them in the output encoding.  This
  includes all characters needed for quotation.

- If no format specifier is found, values are printed at the end of
  the format string in default notation (as if printed with %a).

## Restrictions

- Compiling `printf` with any modern compiler gives you compile time
  warnings about the argument type vs. format string consistency.  If
  these warnings are gone, there are usually no typing problems left.
  For this library, no such warnings can be issued.  And while it is
  type safe, i.e., crashes are more unlikely, particularly the
  argument count va. format specifier count is not checked, while it
  is checked in `printf` with a modern compiler.  With this library,
  printing too many or too few arguments will result in undesirable
  output.

- `%n` is not implemented, because pointers to integers are already
  used for strings, and the ambiguity between 'size_t*' and
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
  the same value multiple times, so `%d %=#x` prints the same value
  decimal and hexadecimal, and `%qs %=p` prints a string in C quotation
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
  `"%c",'\xfe'` prints a replacement characters, because `\xfe` equals
  `(int)0xfffffffe`, which is not valid Unicode, and this library
  has no chance to find out that this is in fact `(char)0xfe`.  So
  printing `'...'` literals is unfortunately broken, without a fix.
  Printing with `%hhc` works as expected (but `%zc` does not,
  because, `\xfe` is an `int`).  Printing `(char)'\xfe'` also works,
  but is more ugly in my opinion (I do not like casts much).

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
  print call -- the compiler throws almost of it away, based on
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
  (less than a few kB, I'd guess).  So while I did try not to mess up,
  this is not optimised for speed.

- Q: Is this safe than `printf`?

  A: Definitely, I think.  There is absolutely no chance to give a
  wrong format specifier and access the stack (like `printf` does via
  `stdarg.h`) in undefined ways.  This is particularly true for
  multi-arch development where with `printf` you need to be careful
  about length specifiers, and you might not get a warning on your
  machine, but the next person will and it will crash there.  I
  usually need to compile for a few times on multiple architectures to
  get the integer length correct, e.g., `%u` vs `%lu` vs. `%llu`
  vs. `%zu`.

  This library's mechanism is also more convenient, because you do not
  need to think much about what you're printing to use the format
  specifier, and there are no `PRId16` etc. macros that obfuscate your
  portable code.  And you can use UTF-8, -16, -32 strings seamlessly
  and mix them freely.  You can print into a malloced or stack
  allocated compound literal safely, with error checking.

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

## How Does This Work?

The main idea is to use macro magic (both standard C99 and some extensions
from gcc, like allowing `__VA_ARGS__` to be empty etc.) to convert the
printf calls:

    x_printf(format)
    x_printf(format, arg1)
    x_printf(format, arg1, arg2)
    ...

Into a recursive call sequence:

    init(STREAM(...), format);
    render(init(STREAM(...), format), arg1)
    render(render(init(&STREAM(...), format), arg1), arg2);
    ...

The `STREAM()` is a temporary stream object, a compound literal, that
is used for state information when parsing the format string, and for
storing the output printer.  The pointer to this temporary object is
returned by all of the functions to the next layer of recursion.  The
`init()` initialises the format parser and the output stream (e.g. for
NUL termination and initial `malloc()`), and each `render()` consumes
one argument by printing it (once or more times) or using it as a
width or precision.

The macro magic is called `VA_REC()`.  You can try it with `gcc -E` or
something:

    VA_REC(f,a)        -> a
    VA_REC(f,a,b)      -> f(a,b)
    VA_REC(f,a,b,c)    -> f(f(a,b),c)
    VA_REC(f,a,b,c,d)  -> f(f(f(a,b),c),d)

The `render()` resolves to a `_Generic()` call that selects the
appropriate printer based on the type of the argument, so that
for each argument, a different C functions may be invoked. E.g.:

    int i;
    render(f,i)    -> print_int(f,i)

    char const *s;
    render(f,s)    -> print_string(f,s)
