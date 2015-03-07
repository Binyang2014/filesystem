TOPDIR = ../../..
TEST_PROGRAM = test

include $(TOPDIR)/GlobalMakefile

objects = basic_structure.o structure_test.o

default:: $(objects)

include $(TOPDIR)/GlobalRules
include $(objects:.$(OBJSUF)=.d)

test:$(object)
	cc -o test -g $(object)
