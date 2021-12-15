/* -*- Mode: C -*- */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "va_print/core.h"
#include "va_print/len.h"
#include "va_print/char.h"
#include "va_print/malloc.h"
#include "va_print/file.h"

#define __unused __attribute__((__unused__))

static void myputc(va_stream_t *s __unused, unsigned c)
{
    if (c == 0) {
        return;
    }
    fputc(c & 0xff, stdout);
}

static va_stream_vtab_t myvtab[1] = {{ .put = myputc }};

__unused
static void test_iuscp(
    unsigned line, char const *f, int a, unsigned b, char const *c, char d, void *e)
{
    printf("%u;%s;", line, f);

    char cf[200];
    strcpy(cf, f);
    for (char *s = cf; *s; s++) {
        if (*s == '~') { *s = '%'; }
    }
    printf(cf, a,b,c,d,e);

    printf(";%s;", va_nprintf(200, f, a,b,c,d,e));

    char s[200] = "test";
    printf("%s;", va_snprintf(s, 200, f, a,b,c,d,e));
    printf("%s;", va_szprintf(s, f, a,b,c,d,e));
    size_t l1 = strlen(s);
    size_t l2 = va_lprintf(f, a,b,c,d,e);
    assert(l1 == l2);

    char *s3 = va_mprintf(va_alloc, f, a,b,c,d,e);
    assert(s3 != NULL);
    printf("%s;", s3);
    free(s3);

    char s4[200];
    va_stream_char_p_t *ss = &VA_STREAM_CHARP(s4, sizeof(s4));
    va_iprintf(ss, f, a,b,c,d,e);
    size_t l4 = ss->pos;
    assert(l1 == l4);
    printf("%s;", s4);

    va_printf(f, a,b,c,d,e);
    printf(";");

    va_pprintf(myvtab, f, a,b,c,d,e);

    printf("\n");
}

extern va_stream_t foo(va_stream_t);

#define TEST_IUSCP(...) test_iuscp(__LINE__, __VA_ARGS__)

#define PRINTF0(W,F,...) va_printf("~u;~s;" W ";" F "\n", __LINE__, F, __VA_ARGS__)

#define PRINTF1(F,W,...) va_printf("~u;~s;~s;" F "\n", __LINE__, F, W, __VA_ARGS__)
#define PRINTF2(W,F,...) PRINTF1(F,W,__VA_ARGS__)

int main(void)
{
    int a __unused = -547;
    unsigned b __unused = 1023;
    void *p __unused = (void*)0x17;
    void *q __unused = NULL;

#if 1
    /* test standard printf formats against libc: */
    va_printf(u"~u;;~c;\u201c\n", __LINE__, 0x201c);
    va_printf(U"~u;;~c;\u201c\n", __LINE__, 0x201c);
    va_printf(u"~u;;~a;\u201c\n", __LINE__, "\u201c");
    va_printf(u"~u;;~v;\u201c\n", __LINE__, "\u201c");
    va_printf(u"~u;;~a;\u201c\n", __LINE__, u"\u201c");
    va_printf(U"~u;;~a;\u201c\n", __LINE__, u"\u201c");
    va_printf(U"~u;;~a;\u201c\n", __LINE__, "\u201c");
    va_printf(u"~u;;~a;\u201c\n", __LINE__, U"\u201c");

    PRINTF1("~c", "\u201c", 0x201c);
    PRINTF1("~a", "\u201c", U"\u201c");

    char16_t a1[20];
    (void)va_szprintf(a1, "test\u201c~qs", "\u201c");
    PRINTF1("test\u201c~qs", a1, "\u201c");

    char32_t a1b[20];
    (void)va_szprintf(a1b, "test\u201c~qs", "\u201c");
    PRINTF1("test\u201c~qs", a1b, "\u201c");

    char16_t *a2 = va_unprintf(20, "test\u201c~qs", "\u201c");
    PRINTF1("test\u201c~qs", a2, "\u201c");

    char32_t *a2b = va_Unprintf(20, "test\u201c~qs", "\u201c");
    PRINTF1("test\u201c~qs", a2b, "\u201c");

    char *a3 = va_mprintf(va_alloc, u"test\u201c~qs", "\u201c");
    PRINTF1("test\u201c~qs", a3, "\u201c");
    free(a3);

    char16_t *a4 = va_umprintf(va_alloc, "test\u201c~qs", "\u201c");
    PRINTF1("test\u201c~qs", a4, "\u201c");
    free(a4);

    char32_t *a4b = va_Umprintf(va_alloc, "test\u201c~qs", "\u201c");
    PRINTF1("test\u201c~qs", a4b, "\u201c");
    free(a4b);

    TEST_IUSCP("Foo: X=~i, [~8x], ~s ~c ~px",  a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~i, [~#8x], ~8s ~c ~p",  a, -1239U, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~c, [~#08x], ~.5s ~c ~p", 'a', -1239U, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~i, [~#-8x], ~#s ~c ~p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~i, [~#-08x], ~-8s ~c ~p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~i, [~#-012x], ~s ~.2c ~p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~i, [~#-0x], ~s ~8c ~#p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~i, [~#-x], ~s ~#c ~p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~i, [~x], ~s ~c ~p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~+i, [~#9o], ~s ~c ~p", 15, 123239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~+i, [~-#9o], ~s ~c ~p", -15, 123439, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~ i, [~9o], ~s ~c ~p", 15, 123239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~ i, [~0o], ~s ~c ~p", 15, 123239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=~i, [~.x], ~s ~c ~p", a, 0, "foo", 'a', p);
    TEST_IUSCP("blah ~#*x, ~s ~c ~p", 5, 178, "foo", 'a', p);
    TEST_IUSCP("blah ~#.*x, ~s ~c ~p", 5, 178, "foo", 'a', p);
    TEST_IUSCP("blah ~#*.*s ~c ~p", 5, 8, "foo", 'a', p);
    TEST_IUSCP("A ~c 0 ~c b ~s ~c ~p", 65, 0x30, "f", 'a', p);
    TEST_IUSCP("A ~hd 0 ~hhu b ~s ~c ~p", 0x123456, 0x123456, "f", 'a', p);
    TEST_IUSCP("A ~hd 0 ~hhu b ~s ~c ~p", -172832, 0x123456, "f", 'a', p);

    /* test special cases that work differently */
    PRINTF1("~.c", "", 'a');
    PRINTF1("d=~d x=~#=x", "d=17 x=0x11", 17);
    PRINTF1("/~s/~s~.s", "/etc/foo", "etc", "foo", 0);
    PRINTF1("/~s/~s~.s", "/etc/foo1", "etc", "foo", 1);

    PRINTF1("~qa", "\"f\\001o\\nou\"", "f\1o\nou");
    PRINTF1("~0qa", "\"f\\001o\\no\\ufffd\\ufffdu\"", "f\1o\no\x81\x82u");

    PRINTF1("~0qa", "\"a\\ufffdg\"", "a\x81g");
    PRINTF1("~0qa", "\"a\\ufffdg\"", "a\xe1\x80g");
    PRINTF1("~0qa", "\"a\\ufffd\\ufffdg\"", "a\xed\xa0g");

    printf("%u;;\ufeffh\uc0c0g\u201ch;%s;\n", __LINE__, "\ufeffh\uc0c0g\u201ch");
    char *s1 = va_nprintf(20, "~a","\ufeffh\uc0c0g\u201ch");
    printf("%u;;\ufeffh\uc0c0g\u201ch;%s;\n", __LINE__, s1);
    char *s2 = va_mprintf(va_alloc, "~a", "\ufeffh\uc0c0g\u201ch");
    printf("%u;;\ufeffh\uc0c0g\u201ch;%s;\n", __LINE__, s2);
    free(s2);

    PRINTF1("~a", "\ufeffh\uc0c0g\u201ch", "\ufeffh\uc0c0g\u201ch");

    PRINTF1(
        "~0qa",
        "\"a\\ufffd\\ufffd\\ufffdb\\ufffdc\\ufffd\\ufffdd\\ufffd\\ufffde\"\n",
        "\x61\xf1\x80\x80\xe1\x80\xc2\x62\x80\x63\x80\xbf\x64\xe0\x9f\x65");

    printf("%u;;\ufeffh\uc0c0g\u201ch;%s;\n", __LINE__, "\ufeffh\uc0c0g\u201ch");

    PRINTF1("~a", "\ufeffh\uc0c0g\u201ch", "\ufeffh\uc0c0g\u201ch");

    PRINTF1("~c", "\u201c", (short)0x201c);
    PRINTF1("~c", "\ua726", (short)0xa726);
    PRINTF1("~c", "\u00d0", (signed char)0xd0);

    PRINTF1("~0qs", "\"f\\001o\\no\\ufffdu\"", "f\1o\no\x81u");
    PRINTF1("~0Qs", "\"f\\u0001o\\no\\ufffdu\"", "f\1o\no\x81u");
    PRINTF1("~0qc", "'f\\001o\\no\\ufffdu'", "f\1o\no\x81u");
    PRINTF1("~0Qc", "'f\\u0001o\\no\\ufffdu'", "f\1o\no\x81u");
    PRINTF1("~#0qa", "f\\001o\\no\\ufffdu", "f\1o\no\x81u");
    PRINTF1("~#0Qa", "f\\u0001o\\no\\ufffdu", "f\1o\no\x81u");
    PRINTF1("~qc", "'A'", 65);
    PRINTF1("~qc", "'\\021'", 17);
    PRINTF1("~Qc", "'\\u0011'", 17);

    PRINTF1("~kc", "foo", "foo");
    PRINTF1("~ks", "foo", "foo");
    PRINTF1("~kc", "'foo foo'", "foo foo");
    PRINTF1("~ks", "'foo foo'", "foo foo");
    PRINTF1("~kc", "'foo '\\''foo'", "foo 'foo");
    PRINTF1("~ks", "'foo '\\''foo'", "foo 'foo");

    PRINTF1("~d", "65", (char)'A');
    PRINTF1("~d", "65", (signed char)'A');
    PRINTF1("~d", "65", 'A');

    PRINTF1("~d", "-1", (char)-1);
    PRINTF1("~d", "-1", (signed char)-1);
    PRINTF1("~d", "-1", (signed short)-1);

    va_error_t e = {0};
    PRINTF1("~0qc", "'\\ufffd\\ufffd\\ufffd'", "\xe0\x90\x80", &e);
    assert(e.code == VA_E_DECODE); /* from UTF-8 input decoding */
    PRINTF1("~0qc", "'\\ufffd'", 0xd97fu, &e);
    assert(e.code == VA_E_ENCODE); /* from quotation */
    PRINTF1("~c", "\ufffd", 0xd97fu, &e);
    assert(e.code == VA_E_ENCODE); /* from UTF-8 output encoding */

    PRINTF2("\"\"", "~1qs", "");

    PRINTF2("16 0x10", "~d ~=#x", 16);

    char const *foo = "fo\no";
    PRINTF0("\"fo\\no\" ~#x", "~qs ~=p", (size_t)foo, foo);
    PRINTF0("~#x \"fo\\no\"", "~p ~=qs", (size_t)foo, foo);

    va_printf("~u;;1016;~x", __LINE__, 16, 16); va_printf("\n");

    char const *abc = "abc";
    PRINTF2("aaba5", "~.1s~=.2s~=.1s~u", &abc, 5);
    PRINTF2("bc5", "~s~u", abc, 5);

    PRINTF2("\"foo\\'bar\"", "~qs", "foo'bar");
    PRINTF2("'\\n'", "~qc", 10);
    PRINTF2("\\020", "~#qc", 16);
    PRINTF2("\\u201c", "~#0qc", 0x201c);
    PRINTF2("\\u201C", "~#0qC", 0x201c);

    PRINTF2("\"foo\\n\"", "~qs", "foo\n");
    PRINTF2("foo\\n", "~#qs", "foo\n");

    PRINTF2("\"foo\\'bar\"", "~Qs", "foo'bar");
    PRINTF2("'\\n'", "~Qc", 10);
    PRINTF2("\\u0010", "~#Qc", 16);
    PRINTF2("\\u201c", "~#0Qc", 0x201c);
    PRINTF2("\\u201C", "~#0QC", 0x201c);

    PRINTF2("ab", "~ks", "ab");
    PRINTF2("'a b'", "~ks", "a b");
    PRINTF2("'a'\\''b'", "~ks", "a'b");
    PRINTF2("a'\\''b", "~#ks", "a'b");

    PRINTF2("-0x5", "~#x", -5);
    PRINTF2("-0x5", "~p", -5);

    PRINTF2("4294945741", "~za", (int)0xffffabcd);
    PRINTF2("43981", "~hza", (int)0xffffabcd);
    PRINTF2("205", "~hhza", (int)0xffffabcd);

    PRINTF2("-1", "~a", (signed long long)-1);
    PRINTF2("-1", "~a", -1);
    PRINTF2("-1", "~a", (short)-1);
    PRINTF2("-1", "~a", (signed char)-1);
    PRINTF2("-1", "~d", (signed long long)-1);
    PRINTF2("-1", "~d", -1);
    PRINTF2("-1", "~d", (short)-1);
    PRINTF2("-1", "~d", (signed char)-1);
    PRINTF2("-1", "~i", (signed long long)-1);
    PRINTF2("-1", "~i", -1);
    PRINTF2("-1", "~i", (short)-1);
    PRINTF2("-1", "~i", (signed char)-1);

    PRINTF2("18446744073709551615", "~u", (signed long long)-1);
    PRINTF2("4294967295", "~u", -1);
    PRINTF2("65535", "~u", (short)-1);
    PRINTF2("255", "~u", (signed char)-1);

    PRINTF2("18446744073709551615", "~d", (unsigned long long)-1);
    PRINTF2("4294967295", "~d", (unsigned)-1);
    PRINTF2("65535", "~d", (unsigned short)-1);
    PRINTF2("255", "~d", (unsigned char)-1);

    PRINTF2("-0x1", "~#x", (signed long long)-1);
    PRINTF2("-0x1", "~#x", -1);
    PRINTF2("-0x1", "~#x", (short)-1);
    PRINTF2("-0x1", "~#x", (signed char)-1);

    PRINTF2("0xffffffffffffffff", "~#zx", (signed long long)-1);
    PRINTF2("0xffffffff", "~#zx", -1);
    PRINTF2("0xffff", "~#zx", (short)-1);
    PRINTF2("0xff", "~#zx", (signed char)-1);

    PRINTF2("0xffffffffffffffff", "~#x", (unsigned long long)-1);
    PRINTF2("0xffffffff", "~#x", (unsigned)-1);
    PRINTF2("0xffff", "~#x", (unsigned short)-1);
    PRINTF2("0xff", "~#x", (unsigned char)-1);

    PRINTF2("CD", "~hhX", 0xabcdU);
    PRINTF2("-0x3211", "~#hx", 0xabcdef);
    PRINTF2("-12817", "~#hd", 0xabcdef);

    PRINTF2("0x12", "~qa", (void*)18);
    PRINTF2("18", "~qa", 18);

    PRINTF2("ab", "a~sb", (char *)NULL, &e);
    assert(e.code == VA_E_NULL);

    PRINTF2("ab", "a~ksb", (char *)NULL, &e);
    assert(e.code == VA_E_NULL);

    PRINTF2("aNULLb", "a~qsb", (char *)NULL, &e);
    assert(e.code == VA_E_OK);

    PRINTF2("anullb", "a~Qsb", (char *)NULL, &e);
    assert(e.code == VA_E_OK);

    PRINTF2("0eba.BC", "~#e.~E", 32, 34);
    PRINTF2("0x41", "~qc", (void*)65);
    PRINTF2("'A'", "~qc", 65);
    PRINTF2("0x41", "~qa", (void*)65);
    PRINTF2("65", "~qa", 65);

    PRINTF2("\"foo\"::char*", "~qs::~=t", "foo", &e);
    PRINTF2("\"foo\"::char16_t*", "~qs::~=t", u"foo", &e);
    PRINTF2("\"foo\"::char32_t*", "~qs::~=t", U"foo", &e);
    PRINTF2("\"foo\"::char32_t*", "~qv::~=t", U"foo", &e);

    PRINTF2("int32_t", "~t", 10);
    PRINTF2("int16_t", "~t", (short)10);
    PRINTF2("uint16_t", "~t", (unsigned short)10);
    PRINTF2("void*", "~t", NULL);
    PRINTF2("void* x=0x10", "~t x=~=qza", (void*)16);
    PRINTF2("char* x=\"foo\"", "~t x=~=qza", "foo");
    PRINTF2("int32_t x=7", "~t x=~=qza", 7);
    PRINTF2("char16_t* x=u\"foo\"", "~t x=~=qza", u"foo");

    PRINTF2("\"\\n\"", "~qzs", "\n");
    PRINTF2("u\"\\n\"", "~qzs", u"\n");
    PRINTF2("U\"\\n\"", "~qzs", U"\n");
    PRINTF2("\"\\n\"", "~qu", "\n");
    PRINTF2("u\"\\n\"", "~qu", u"\n");
    PRINTF2("U\"\\n\"", "~qu", U"\n");
    PRINTF2("'\\n'", "~qzc", (char)10);
    PRINTF2("u'\\n'", "~qzc", (short)10);
    PRINTF2("U'\\n'", "~qzc", 10);
    PRINTF2("U'\\n'", "~qzc", (long long)10);

    PRINTF2("255", "~hhu", -1);

    PRINTF2("NULL  a", "~-6qsa", (char*)NULL);
    PRINTF2("null  a", "~-6Qsa", (char*)NULL);
    PRINTF2("   NULLa", "~7qsa", (char*)NULL);
    PRINTF2("   nulla", "~7Qsa", (char*)NULL);
    PRINTF2("NULL  a", "~*qsa", -6, (char*)NULL);
    PRINTF2("null  a", "~*Qsa", -6, (char*)NULL);
    PRINTF2("NULL  a", "~-*qsa",  6ULL, (char*)NULL);
    PRINTF2("null  a", "~-*Qsa",  6ULL, (char*)NULL);
    PRINTF2("   NULLa", "~*qsa", 7, (char*)NULL);
    PRINTF2("   nulla", "~*Qsa", 7, (char*)NULL);
    PRINTF2("NULLa", "~-2.qsa", (char*)NULL);
    PRINTF2("nulla", "~-2.Qsa", (char*)NULL);
    PRINTF2("NULLa", "~-2.8qsa", (char*)NULL);
    PRINTF2("nulla", "~-2.8Qsa", (char*)NULL);
    PRINTF2("NULLa", "~-4.2qsa", (char*)NULL);
    PRINTF2("nulla", "~-4.2Qsa", (char*)NULL);

    PRINTF2("\"j\"   a", "~-6qsa", "j");
    PRINTF2("\"j\"   a", "~-6Qsa", "j");
    PRINTF2("    \"j\"a", "~7qsa", "j");
    PRINTF2("    \"j\"a", "~7Qsa", "j");
    PRINTF2("\"j\"   a", "~*qsa", -6, "j");
    PRINTF2("\"j\"   a", "~*Qsa", -6, "j");
    PRINTF2("\"j\"   a", "~-*qsa",  6ULL, "j");
    PRINTF2("\"j\"   a", "~-*Qsa",  6ULL, "j");
    PRINTF2("    \"j\"a", "~*qsa", 7, "j");
    PRINTF2("    \"j\"a", "~*Qsa", 7, "j");
    PRINTF2("\"\"a", "~-2.qsa", "j");
    PRINTF2("\"\"a", "~-2.Qsa", "j");
    PRINTF2("\"\" a", "~-3.qsa", "j");
    PRINTF2("\"\" a", "~-3.Qsa", "j");
    PRINTF2("\"j\"a", "~-2.8qsa", "j");
    PRINTF2("\"j\"a", "~-2.8Qsa", "j");
    PRINTF2("\"j\" a", "~-4.2qsa", "j");
    PRINTF2("\"j\" a", "~-4.2Qsa", "j");

    PRINTF2("\"j\"   a", "~-6qsa", u"j");
    PRINTF2("\"j\"   a", "~-6Qsa", U"j");
    PRINTF2("    \"j\"a", "~7qsa", u"j");
    PRINTF2("    \"j\"a", "~7Qsa", U"j");
    PRINTF2("\"j\"   a", "~*qsa", -6, u"j");
    PRINTF2("\"j\"   a", "~*Qsa", -6, U"j");
    PRINTF2("\"j\"   a", "~-*qsa",  6ULL, u"j");
    PRINTF2("\"j\"   a", "~-*Qsa",  6ULL, U"j");
    PRINTF2("    \"j\"a", "~*qsa", 7, u"j");
    PRINTF2("    \"j\"a", "~*Qsa", 7, U"j");
    PRINTF2("\"\"a", "~-2.qsa", u"j");
    PRINTF2("\"\"a", "~-2.Qsa", U"j");
    PRINTF2("\"\" a", "~-3.qsa", u"j");
    PRINTF2("\"\" a", "~-3.Qsa", U"j");
    PRINTF2("\"j\"a", "~-2.8qsa", u"j");
    PRINTF2("\"j\"a", "~-2.8Qsa", U"j");
    PRINTF2("\"j\" a", "~-4.2qsa", u"j");
    PRINTF2("\"j\" a", "~-4.2Qsa", U"j");

    PRINTF2("u\"j\"  a", "~-6qzsa", u"j");
    PRINTF2("\"j\"   a", "~-6Qzsa", U"j");
    PRINTF2("   U\"j\"a", "~7qzsa", U"j");
    PRINTF2("    \"j\"a", "~7Qzsa", u"j");
    PRINTF2("u\"j\"  a", "~*qzsa", -6, u"j");
    PRINTF2("\"j\"   a", "~*Qzsa", -6, U"j");
    PRINTF2("U\"j\"  a", "~-*qzsa",  6ULL, U"j");
    PRINTF2("\"j\"   a", "~-*Qzsa",  6ULL, u"j");
    PRINTF2("   u\"j\"a", "~*qzsa", 7, u"j");
    PRINTF2("    \"j\"a", "~*Qzsa", 7, U"j");
    PRINTF2("u\"\"a", "~-2.qzsa", u"j");
    PRINTF2("\"\"a", "~-2.Qzsa", U"j");
    PRINTF2("U\"\"a", "~-3.qzsa", U"j");
    PRINTF2("\"\" a", "~-3.Qzsa", u"j");
    PRINTF2("u\"j\"a", "~-2.8qzsa", u"j");
    PRINTF2("\"j\"a", "~-2.8Qzsa", U"j");
    PRINTF2("u\"j\"a", "~-4.2qzsa", u"j");
    PRINTF2("\"j\" a", "~-4.2Qzsa", U"j");

    PRINTF2("   \"abc\",", "~8qzv,", "abc");
    PRINTF2("  u\"abc\",", "~8qzv,", u"abc");
    PRINTF2("  U\"abc\",", "~8qzv,", U"abc");
    PRINTF2("   \"abc\",", "~8qu,", "abc");
    PRINTF2("  u\"abc\",", "~8qu,", u"abc");
    PRINTF2("  U\"abc\",", "~8qu,", U"abc");

    PRINTF2("     \'A\',", "~8qzc,", (char)65);
    PRINTF2("    u\'A\',", "~8qzc,", (short)65);
    PRINTF2("    U\'A\',", "~8qzc,", (int)65);
    PRINTF2("    U\'A\',", "~8qzc,", (long)65);
    PRINTF2("    U\'A\',", "~8qzc,", (long long)65);
    PRINTF2("     \'A\',", "~8qzc,", (unsigned char)65);
    PRINTF2("    u\'A\',", "~8qzc,", (unsigned short)65);
    PRINTF2("    U\'A\',", "~8qzc,", (unsigned int)65);
    PRINTF2("    U\'A\',", "~8qzc,", (unsigned long)65);
    PRINTF2("    U\'A\',", "~8qzc,", (unsigned long long)65);

    PRINTF2("char32_t*  x =     U\"j\",", "~-10t x = ~=8qzv,", U"j");
    PRINTF2("char16_t*  x =     u\"j\",", "~-10t x = ~=8qzv,", u"j");
    PRINTF2("char*      x =      abc,", "~-10t x = ~=8zv,", "abc");
    PRINTF2("char*      x =    \"abc\",", "~-10t x = ~=8qzv,", "abc");
    PRINTF2("int8_t     x =      \'c\',", "~-10t x = ~=8qzc,", (signed char)'c');
    PRINTF2("int16_t    x =     u\'c\',", "~-10t x = ~=8qzc,", (short)'c');
    PRINTF2("int32_t    x =     U\'c\',", "~-10t x = ~=8qzc,", (int)'c');
    PRINTF2("int64_t    x =     U\'c\',", "~-10t x = ~=8qzc,", (long long)'c');
    PRINTF2("uint8_t    x =      \'c\',", "~-10t x = ~=8qzc,", (unsigned char)'c');
    PRINTF2("uint16_t   x =     u\'c\',", "~-10t x = ~=8qzc,", (unsigned short)'c');
    PRINTF2("uint32_t   x =     U\'c\',", "~-10t x = ~=8qzc,", (unsigned)'c');
    PRINTF2("uint64_t   x =     U\'c\',", "~-10t x = ~=8qzc,", (unsigned long long)'c');

    PRINTF2("\"\\ufffd\"", "~0qa", u"\xd801");
#endif

    return 0;
}
