TOPDIR = ../..
TEST_PROGRAM = bitmap_test hash_test err_test

include $(TOPDIR)/GlobalMakefile
object = bitmap.o hash.o errinfo.o
target = $(TOPDIR)/lib/libtool.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^

default:: $(object)

bitmap_test: bitmap_test.o $(object)
	$(CC) $(CFLAGS) -o $@ $^
hash_test: hash_test.o hash.o
	$(CC) $(CFLAGS) -o $@ $^
err_test: err_test.o errinfo.o
	$(CC) $(CFLAGS) -o $@ $^

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

ifeq ($(MAKECMDGOALS),bitmap_test)
include bitmap_test.d
endif

ifeq ($(MAKECMDGOALS),hash_test)
include hash_test.d
endif

ifeq ($(MAKECMDGOALS),err_test)
include err_test.d
endif
