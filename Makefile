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
        $(CFLAGS.opt) \
        $(MORE_CFLAGS))

CPPFLAGS.dep := \
    -MMD -MP

CPPFLAGS.inc := \
    -I./include

CPPFLAGS := \
    $(filter-out $(NO_FLAGS), \
        $(CPPFLAGS.dep) \
        $(CPPFLAGS.inc) \
        $(MORE_CPPFLAGS))

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
    out/libvastringify.a \
    out/test1.x \
    out/test3.x

test: \
    out/test2-readme.o

out/test2-readme.c: \
    src/test2-template.c \
    README.md \
    example2c.pl
	perl ./example2c.pl $< $@

out/test1.x: out/test1.o out/libvastringify.a
out/test3.x: out/test3.o out/libvastringify.a

LIB_O := \
    out/core.o \
    out/len.o \
    out/char.o \
    out/char16.o \
    out/char32.o \
    out/char_utf8.o \
    out/char_utf16.o \
    out/char_utf32.o \
    out/alloc.o \
    out/alloc16.o \
    out/alloc32.o \
    out/alloc_utf8.o \
    out/alloc_utf16.o \
    out/alloc_utf32.o \
    out/alloc_compat.o \
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

out/libvastringify.a: $(LIB_O)

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
	mkdir -p $(DESTDIR)$(includedir)/va_print
	cd include/va_print && for i in *.h; do \
	    install -m 644 $$i $(DESTDIR)$(includedir)/va_print/$$i; \
	done
	mkdir -p $(DESTDIR)$(libdir)
	install -m 644 out/libvastringify.a $(DESTDIR)$(libdir)/libvastringify.a

.PHONY: uninstall
uninstall:
	rm -rf $(DESTDIR)$(includedir)/va_print
	rm -f $(DESTDIR)$(libdir)/libvastringify.a

.PHONY: clean
clean:
	rm -rf out

.PHONY: distclean
distclean: clean

.PHONY: test
test: test1 test3

.PHONY: test1
test1: all
	$(EXECUTE) ./out/test1.x > test.out
	cat test.out
	perl -n cmp.pl test.out

.PHONY: test3
test3: all
	$(EXECUTE) ./out/test3.x

.PHONY: stack
stack: $(LIB_O:.o=.png) $(LIB_O:.o=.dot)  $(LIB_O:.o=.dot)

out/%.dot: out/%.s
	./stack.pl $<

%.png: %.dot
	dot -Tpng $< -o $@

-include out/*.d
