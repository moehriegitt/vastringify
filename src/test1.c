/* -*- Mode: C -*- */

#include <stdio.h>
#include <string.h>
#include <assert.h>
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
    printf(f, a,b,c,d,e);

    printf(";%s;", va_nprintf(200, f, a,b,c,d,e));

    char s[200] = "test";
    printf("%s;", va_snprintf(s, 200, f, a,b,c,d,e));
    printf("%s;", va_szprintf(s, f, a,b,c,d,e));
    size_t l1 = strlen(s);
    size_t l2 = va_lprintf(f, a,b,c,d,e);
    assert(l1 == l2);

    char *s3 = va_mprintf(realloc, f, a,b,c,d,e);
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

int main(void)
{
    int a __unused = -547;
    unsigned b __unused = 1023;
    void *p __unused = (void*)0x17;
    void *q __unused = NULL;

#if 1
    /* test standard printf formats against libc: */
    va_printf("%u;;%c;\u201c\n", __LINE__, 0x201c);
    va_printf(u"%u;;%c;\u201c\n", __LINE__, 0x201c);
    va_printf(U"%u;;%c;\u201c\n", __LINE__, 0x201c);
    va_printf(u"%u;;%a;\u201c\n", __LINE__, "\u201c");
    va_printf(u"%u;;%a;\u201c\n", __LINE__, u"\u201c");
    va_printf(U"%u;;%a;\u201c\n", __LINE__, u"\u201c");
    va_printf(U"%u;;%a;\u201c\n", __LINE__, "\u201c");
    va_printf(u"%u;;%a;\u201c\n", __LINE__, U"\u201c");
    va_printf("%u;;%a;\u201c\n", __LINE__, U"\u201c");

    char16_t a1[20];
    (void)va_szprintf(a1, "test\u201c%qs", "\u201c");
    va_printf("%u;;%s;test\u201c%qs\n", __LINE__, a1, "\u201c");

    char32_t a1b[20];
    (void)va_szprintf(a1b, "test\u201c%qs", "\u201c");
    va_printf("%u;;%s;test\u201c%qs\n", __LINE__, a1b, "\u201c");

    char16_t *a2 = va_unprintf(20, "test\u201c%qs", "\u201c");
    va_printf("%u;;%s;test\u201c%qs\n", __LINE__, a2, "\u201c");

    char32_t *a2b = va_Unprintf(20, "test\u201c%qs", "\u201c");
    va_printf("%u;;%s;test\u201c%qs\n", __LINE__, a2b, "\u201c");

    char *a3 = va_mprintf(realloc, u"test\u201c%qs", "\u201c");
    va_printf("%u;;%s;test\u201c%qs\n", __LINE__, a3, "\u201c");
    free(a3);

    char16_t *a4 = va_umprintf(realloc, "test\u201c%qs", "\u201c");
    va_printf("%u;;%s;test\u201c%qs\n", __LINE__, a4, "\u201c");
    free(a4);

    char32_t *a4b = va_Umprintf(realloc, "test\u201c%qs", "\u201c");
    va_printf("%u;;%s;test\u201c%qs\n", __LINE__, a4b, "\u201c");
    free(a4b);

    TEST_IUSCP("Foo: X=%i, [%8x], %s %c %px",  a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%i, [%#8x], %8s %c %p",  a, -1239U, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%c, [%#08x], %.5s %c %p", 'a', -1239U, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%i, [%#-8x], %#s %c %p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%i, [%#-08x], %-8s %c %p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%i, [%#-012x], %s %.2c %p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%i, [%#-0x], %s %8c %#p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%i, [%#-x], %s %#c %p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%i, [%x], %s %c %p", a, 1239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%+i, [%#9o], %s %c %p", 15, 123239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%+i, [%-#9o], %s %c %p", -15, 123439, "foo", 'a', p);
    TEST_IUSCP("Foo: X=% i, [%9o], %s %c %p", 15, 123239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=% i, [%0o], %s %c %p", 15, 123239, "foo", 'a', p);
    TEST_IUSCP("Foo: X=%i, [%.x], %s %c %p", a, 0, "foo", 'a', p);
    TEST_IUSCP("blah %#*x, %s %c %p", 5, 178, "foo", 'a', p);
    TEST_IUSCP("blah %#.*x, %s %c %p", 5, 178, "foo", 'a', p);
    TEST_IUSCP("blah %#*.*s %c %p", 5, 8, "foo", 'a', p);
    TEST_IUSCP("A %c 0 %c b %s %c %p", 65, 0x30, "f", 'a', p);
    TEST_IUSCP("A %hd 0 %hhu b %s %c %p", 0x123456, 0x123456, "f", 'a', p);
    TEST_IUSCP("A %hd 0 %hhu b %s %c %p", -172832, 0x123456, "f", 'a', p);

    /* test special cases that work differently */
    va_printf("%u;;%.c;\n", __LINE__, 'a');
    va_printf("%u;;d=%d x=%#=x;d=17 x=0x11\n", __LINE__, 17);
    va_printf("%u;;/%s/%s%.s;/etc/foo\n", __LINE__, "etc", "foo", 0);
    va_printf("%u;;/%s/%s%.s;/etc/foo1\n", __LINE__, "etc", "foo", 1);

    va_printf("%u;;%qa;\"f\\001o\\nou\"\n", __LINE__, "f\1o\nou");
    va_printf("%u;;%0qa;\"f\\001o\\no\\ufffd\\ufffdu\"\n", __LINE__, "f\1o\no\x81\x82u");

    va_printf("%u;;%0qa;\"a\\ufffdg\"\n", __LINE__, "a\x81g");
    va_printf("%u;;%0qa;\"a\\ufffdg\"\n", __LINE__, "a\xe1\x80g");
    va_printf("%u;;%0qa;\"a\\ufffd\\ufffdg\"\n", __LINE__, "a\xed\xa0g");

    printf("%u;;\ufeffh\uc0c0g\u201ch;%s;\n", __LINE__, "\ufeffh\uc0c0g\u201ch");
    char *s1 = va_nprintf(20, "%a","\ufeffh\uc0c0g\u201ch");
    printf("%u;;\ufeffh\uc0c0g\u201ch;%s;\n", __LINE__, s1);
    char *s2 = va_mprintf(realloc, "%a", "\ufeffh\uc0c0g\u201ch");
    printf("%u;;\ufeffh\uc0c0g\u201ch;%s;\n", __LINE__, s2);
    free(s2);
    va_printf("%u;;\ufeffh\uc0c0g\u201ch;%a;\n", __LINE__, "\ufeffh\uc0c0g\u201ch");

    va_printf(
        "%u;;%0qa;\"a\\ufffd\\ufffd\\ufffdb\\ufffdc\\ufffd\\ufffdd\\ufffd\\ufffde\"\n",
        __LINE__,
        "\x61\xf1\x80\x80\xe1\x80\xc2\x62\x80\x63\x80\xbf\x64\xe0\x9f\x65");

    printf("%u;;\ufeffh\uc0c0g\u201ch;%s;\n", __LINE__, "\ufeffh\uc0c0g\u201ch");
    va_printf("%u;;\ufeffh\uc0c0g\u201ch;%a;\n", __LINE__, "\ufeffh\uc0c0g\u201ch");

    va_printf("%u;;\u201c;%c\n", __LINE__, (short)0x201c);
    va_printf("%u;;\ua726;%c\n", __LINE__, (short)0xa726);
    va_printf("%u;;\u00d0;%c\n", __LINE__, (signed char)0xd0);

    va_printf("%u;;%0qs;\"f\\001o\\no\\ufffdu\"\n", __LINE__, "f\1o\no\x81u");
    va_printf("%u;;%0Qs;\"f\\u0001o\\no\\ufffdu\"\n", __LINE__, "f\1o\no\x81u");
    va_printf("%u;;%0qc;'f\\001o\\no\\ufffdu'\n", __LINE__, "f\1o\no\x81u");
    va_printf("%u;;%0Qc;'f\\u0001o\\no\\ufffdu'\n", __LINE__, "f\1o\no\x81u");
    va_printf("%u;;%#0qa;f\\001o\\no\\ufffdu\n", __LINE__, "f\1o\no\x81u");
    va_printf("%u;;%#0Qa;f\\u0001o\\no\\ufffdu\n", __LINE__, "f\1o\no\x81u");
    va_printf("%u;;%qc;'A'\n", __LINE__, 65);
    va_printf("%u;;%qc;'\\021'\n", __LINE__, 17);
    va_printf("%u;;%Qc;'\\u0011'\n", __LINE__, 17);

    va_printf("%u;;%kc;foo\n", __LINE__, "foo");
    va_printf("%u;;%ks;foo\n", __LINE__, "foo");
    va_printf("%u;;%kc;'foo foo'\n", __LINE__, "foo foo");
    va_printf("%u;;%ks;'foo foo'\n", __LINE__, "foo foo");
    va_printf("%u;;%kc;'foo '\\''foo'\n", __LINE__, "foo 'foo");
    va_printf("%u;;%ks;'foo '\\''foo'\n", __LINE__, "foo 'foo");

    va_printf("%u;;%d;65\n", __LINE__, (char)'A');
    va_printf("%u;;%d;65\n", __LINE__, (signed char)'A');
    va_printf("%u;;%d;65\n", __LINE__, 'A');

    va_printf("%u;;%d;-1\n", __LINE__, (char)-1);
    va_printf("%u;;%d;-1\n", __LINE__, (signed char)-1);
    va_printf("%u;;%d;-1\n", __LINE__, (signed short)-1);

    va_error_t e = {0};
    va_printf("%u;;%0qc;'\\ufffd\\ufffd\\ufffd'\n", __LINE__, "\xe0\x90\x80", &e);
    assert(e.code == VA_E_DECODE); /* from UTF-8 input decoding */
    va_printf("%u;;%0qc;'\\ufffd'\n", __LINE__, 0xd97fu, &e);
    assert(e.code == VA_E_ENCODE); /* from quotation */
    va_printf("%u;;%c;\ufffd\n", __LINE__, 0xd97fu, &e);
    assert(e.code == VA_E_ENCODE); /* from UTF-8 output encoding */

    va_printf("%u;;\"\";%1qs\n", __LINE__, "");

    va_printf("%u;;16 0x10;%d %=#x\n", __LINE__, 16);

    char const *foo = "fo\no";
    va_printf("%u;;\"fo\\no\" %#x;%qs %=p\n", __LINE__, (size_t)foo, foo);
    va_printf("%u;;%#x \"fo\\no\";%p %=qs\n", __LINE__, (size_t)foo, foo);
    va_printf("%u;;1016;%x", __LINE__, 16, 16); va_printf("\n");
    char const *abc = "abc";
    va_printf("%u;;aaba5;%.1s%=.2s%=.1s%u\n", __LINE__, &abc, 5);
    va_printf("%u;;bc5;%s%u\n", __LINE__, abc, 5);

    va_printf("%u;;\"foo\\'bar\";%qs\n", __LINE__, "foo'bar");
    va_printf("%u;;'\\n';%qc\n", __LINE__, 10);
    va_printf("%u;;\\020;%#qc\n", __LINE__, 16);
    va_printf("%u;;\\u201c;%#0qc\n", __LINE__, 0x201c);
    va_printf("%u;;\\u201C;%#0qC\n", __LINE__, 0x201c);

    va_printf("%u;;\"foo\\'bar\";%Qs\n", __LINE__, "foo'bar");
    va_printf("%u;;'\\n';%Qc\n", __LINE__, 10);
    va_printf("%u;;\\u0010;%#Qc\n", __LINE__, 16);
    va_printf("%u;;\\u201c;%#0Qc\n", __LINE__, 0x201c);
    va_printf("%u;;\\u201C;%#0QC\n", __LINE__, 0x201c);

    va_printf("%u;;ab;%ks\n", __LINE__, "ab");
    va_printf("%u;;'a b';%ks\n", __LINE__, "a b");
    va_printf("%u;;'a'\\''b';%ks\n", __LINE__, "a'b");
    va_printf("%u;;a'\\''b;%#ks\n", __LINE__, "a'b");

    va_printf("%u;;-0x5;%#x\n", __LINE__, -5);
    va_printf("%u;;-0x5;%p\n", __LINE__, -5);

    va_printf("%u;;4294945741;%za\n", __LINE__, (int)0xffffabcd);
    va_printf("%u;;43981;%hza\n", __LINE__, (int)0xffffabcd);
    va_printf("%u;;205;%hhza\n", __LINE__, (int)0xffffabcd);

    va_printf("%u;;-1;%a\n", __LINE__, (signed long long)-1);
    va_printf("%u;;-1;%a\n", __LINE__, -1);
    va_printf("%u;;-1;%a\n", __LINE__, (short)-1);
    va_printf("%u;;-1;%a\n", __LINE__, (signed char)-1);
    va_printf("%u;;-1;%d\n", __LINE__, (signed long long)-1);
    va_printf("%u;;-1;%d\n", __LINE__, -1);
    va_printf("%u;;-1;%d\n", __LINE__, (short)-1);
    va_printf("%u;;-1;%d\n", __LINE__, (signed char)-1);
    va_printf("%u;;-1;%i\n", __LINE__, (signed long long)-1);
    va_printf("%u;;-1;%i\n", __LINE__, -1);
    va_printf("%u;;-1;%i\n", __LINE__, (short)-1);
    va_printf("%u;;-1;%i\n", __LINE__, (signed char)-1);

    va_printf("%u;;18446744073709551615;%u\n", __LINE__, (signed long long)-1);
    va_printf("%u;;4294967295;%u\n", __LINE__, -1);
    va_printf("%u;;65535;%u\n", __LINE__, (short)-1);
    va_printf("%u;;255;%u\n", __LINE__, (signed char)-1);

    va_printf("%u;;18446744073709551615;%d\n", __LINE__, (unsigned long long)-1);
    va_printf("%u;;4294967295;%d\n", __LINE__, (unsigned)-1);
    va_printf("%u;;65535;%d\n", __LINE__, (unsigned short)-1);
    va_printf("%u;;255;%d\n", __LINE__, (unsigned char)-1);

    va_printf("%u;;-0x1;%#x\n", __LINE__, (signed long long)-1);
    va_printf("%u;;-0x1;%#x\n", __LINE__, -1);
    va_printf("%u;;-0x1;%#x\n", __LINE__, (short)-1);
    va_printf("%u;;-0x1;%#x\n", __LINE__, (signed char)-1);

    va_printf("%u;;0xffffffffffffffff;%#zx\n", __LINE__, (signed long long)-1);
    va_printf("%u;;0xffffffff;%#zx\n", __LINE__, -1);
    va_printf("%u;;0xffff;%#zx\n", __LINE__, (short)-1);
    va_printf("%u;;0xff;%#zx\n", __LINE__, (signed char)-1);

    va_printf("%u;;0xffffffffffffffff;%#x\n", __LINE__, (unsigned long long)-1);
    va_printf("%u;;0xffffffff;%#x\n", __LINE__, (unsigned)-1);
    va_printf("%u;;0xffff;%#x\n", __LINE__, (unsigned short)-1);
    va_printf("%u;;0xff;%#x\n", __LINE__, (unsigned char)-1);

    va_printf("%u;;CD;%hhX\n", __LINE__, 0xabcdU);
    va_printf("%u;;-0x3211;%#hx\n", __LINE__, 0xabcdef);
    va_printf("%u;;-12817;%#hd\n", __LINE__, 0xabcdef);

    va_printf("%u;;0x12;%qa\n", __LINE__, (void*)18);
    va_printf("%u;;18;%qa\n", __LINE__, 18);

    va_printf("%u;;ab;a%sb\n", __LINE__, (char *)NULL, &e);
    assert(e.code == VA_E_NULL);

    va_printf("%u;;ab;a%ksb\n", __LINE__, (char *)NULL, &e);
    assert(e.code == VA_E_NULL);

    va_printf("%u;;aNULLb;a%qsb\n", __LINE__, (char *)NULL, &e);
    assert(e.code == VA_E_OK);

    va_printf("%u;;anullb;a%Qsb\n", __LINE__, (char *)NULL, &e);
    assert(e.code == VA_E_OK);

    va_printf("%u;;0eba.BC;%#e.%E\n", __LINE__, 32, 34);
    va_printf("%u;;0x41;%qc\n", __LINE__, (void*)65);
    va_printf("%u;;'A';%qc\n", __LINE__, 65);
    va_printf("%u;;0x41;%qa\n", __LINE__, (void*)65);
    va_printf("%u;;65;%qa\n", __LINE__, 65);

    va_printf("%u;;\"foo\"::char*;%qs::%=t\n", __LINE__, "foo", &e);
    va_printf("%u;;\"foo\"::char16_t*;%qs::%=t\n", __LINE__, u"foo", &e);
    va_printf("%u;;\"foo\"::char32_t*;%qs::%=t\n", __LINE__, U"foo", &e);

    va_printf("%u;;int32_t;%t\n", __LINE__, 10);
    va_printf("%u;;int16_t;%t\n", __LINE__, (short)10);
    va_printf("%u;;uint16_t;%t\n", __LINE__, (unsigned short)10);
    va_printf("%u;;void*;%t\n", __LINE__, NULL);
    va_printf("%u;;void* x=0x10;%t x=%=qza\n", __LINE__, (void*)16);
    va_printf("%u;;char* x=\"foo\";%t x=%=qza\n", __LINE__, "foo");
    va_printf("%u;;int32_t x=7;%t x=%=qza\n", __LINE__, 7);
    va_printf("%u;;char16_t* x=u\"foo\";%t x=%=qza\n", __LINE__, u"foo");

    va_printf("%u;;\"\\n\";%qzs\n", __LINE__, "\n");
    va_printf("%u;;u\"\\n\";%qzs\n", __LINE__, u"\n");
    va_printf("%u;;U\"\\n\";%qzs\n", __LINE__, U"\n");
    va_printf("%u;;'\\n';%qzc\n", __LINE__, (char)10);
    va_printf("%u;;u'\\n';%qzc\n", __LINE__, (short)10);
    va_printf("%u;;U'\\n';%qzc\n", __LINE__, 10);
    va_printf("%u;;U'\\n';%qzc\n", __LINE__, (long long)10);

    va_printf("%u;;255;%hhu\n", __LINE__, -1);
#endif

    return 0;
}
