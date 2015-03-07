TOPDIR = ../..
TEST_PROGRAM = bitmap_test

include $(TOPDIR)/GlobalMakefile
object = bitmap.o

default:: $(object)

bitmap_test: bitmap_test.o $(object)
	cc -g -o bitmap_test bitmap_test.o $(object)

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

ifeq ($(MAKECMDGOALS),bitmap_test)
include bitmap_test.d
endif
