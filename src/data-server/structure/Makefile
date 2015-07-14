TOPDIR = ../../..
TESTDIR = ./test

include $(TOPDIR)/GlobalMakefile
objects = basic_structure.o
target = $(TOPDIR)/lib/libvfs.a

$(target): $(objects)
	$(AR) $(ARFLAG) $@ $^

default:: $(objects)

test::
	cd $(TESTDIR) &&  make || exit 1;

include $(TOPDIR)/GlobalRules

include $(objects:.$(OBJSUF)=.d)
