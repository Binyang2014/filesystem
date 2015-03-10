TOPDIR = ../..
TEST_PROGRAM = bitmap_test hash_test

include $(TOPDIR)/GlobalMakefile
object = bitmap.o hash.o

default:: $(object)

bitmap_test: bitmap_test.o $(object)
	cc -g -o bitmap_test bitmap_test.o $(object)
hash_test: hash_test.o hash.o
	cc -g -o $@ $^

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

ifeq ($(MAKECMDGOALS),bitmap_test)
include bitmap_test.d
endif

ifeq ($(MAKECMDGOALS),hash_test)
include hash_test.d
endif
