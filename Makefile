# -*- Mode: Makefile -*-

CC := gcc

CFLAGS.warn := \
    -W -Wall -Wextra \
    -Wpadded \
    -Wconversion -Wsign-conversion \
    -Winit-self \
    -Wuninitialized \
    -Wfloat-equal \
    -Wshadow \
    -Wpointer-arith \
    -Wundef \
    -Wcast-align \
    -Wwrite-strings \
    -Wlogical-op \
    -Waggregate-return \
    -Wstrict-prototypes \
    -Wold-style-definition \
    -Wmissing-prototypes \
    -Wmissing-declarations \
    -Wredundant-decls \
    -Wnested-externs \
    -Wvla \
    -Wno-format

CFLAGS.err := \
    -Werror=incompatible-pointer-types

CFLAGS.opt := \
    -Os \
    -fconserve-stack \
    -fstack-usage

CFLAGS.debug := \
    -g3

CFLAGS.safe := \
    -fno-strict-aliasing \
    -fno-delete-null-pointer-checks \
    -fwrapv

CFLAGS.misc := \
    -save-temps=obj

CFLAGS.sanitize :=

CFLAGS := \
    $(filter-out $(NO_FLAGS), \
        $(CFLAGS.misc) \
        $(CFLAGS.safe) \
        $(CFLAGS.sanitize) \
        $(CFLAGS.warn) \
        $(CFLAGS.err) \
        $(CFLAGS.debug) \
        $(CFLAGS.opt))

CPPFLAGS.dep := \
    -MMD -MP

CPPFLAGS.inc := \
    -I./include

CPPFLAGS := \
    $(filter-out $(NO_FLAGS), \
        $(CPPFLAGS.dep) \
        $(CPPFLAGS.inc))

LDFLAGS := \
    -L./out

LDLIBS :=

AR := \
    ar

ARFLAGS := \
    rcD

prefix := /usr/local
exec_prefix := ${prefix}
includedir := ${prefix}/include
libdir := ${exec_prefix}/lib

all:
.PHONY: all

.SECONDARY:
.SUFFIXES:

all: \
    out/libvastringify.a

test: \
    out/test2-readme.o \
    out/test1.x

out/test2-readme.c: \
    src/test2-template.c \
    README.md \
    example2c.pl
	perl ./example2c.pl $< $@

out/test1.x: out/test1.o out/libvastringify.a

out/libvastringify.a: \
    out/core.o \
    out/len.o \
    out/char.o \
    out/char16.o \
    out/char32.o \
    out/char_utf8.o \
    out/char_utf16.o \
    out/char_utf32.o \
    out/malloc.o \
    out/malloc16.o \
    out/malloc32.o \
    out/malloc_utf8.o \
    out/malloc_utf16.o \
    out/malloc_utf32.o \
    out/file.o \
    out/file16be.o \
    out/file16le.o \
    out/file32be.o \
    out/file32le.o \
    out/file_utf8.o \
    out/file_utf16be.o \
    out/file_utf16le.o \
    out/file_utf32be.o \
    out/file_utf32le.o \
    out/utf8.o \
    out/utf16.o \
    out/utf32.o

out/%.x:
	$(CC) $(CFLAGS) $(filter-out %.a,$+) $(LDFLAGS) $(LDLIBS) \
	    $(patsubst lib%.a,-l%,$(notdir $(filter %.a,$+))) \
	    -o $@

out/%.a:
	$(AR) $(ARFLAGS) $@ $+

out/%.o: src/%.c
	@mkdir -p out
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -c -o $@

%.o: %.c
	@mkdir -p out
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -c -o $@

.PHONY: install
install: all
	test -n "$(prefix)"
	test -n "$(libdir)"
	test -n "$(includedir)"
	mkdir -p $(DESTDIR)$(includedir)
	install -m 644 include/va_stringify.h $(DESTDIR)$(includedir)/va_stringify.h
	mkdir -p $(DESTDIR)$(libdir)
	install -m 644 out/libvastringify.a $(DESTDIR)$(libdir)/libvastringify.a

.PHONY: clean
clean:
	rm -rf out

.PHONY: distclean
distclean: clean

.PHONY: test
test: all
	$(EXECUTE) ./out/test1.x > test.out
	cat test.out
	perl -n cmp.pl test.out

-include out/*.d