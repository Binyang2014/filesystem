TOPDIR = ../../..
TEST_PROGRAM = test

include $(TOPDIR)/GlobalMakefile

objects = test.o dataserver.o

default:: $(objects)

include $(TOPDIR)/GlobalRules
include $(objects:.o=.d)

test:$(objects)
	cc -o test $(objects)
