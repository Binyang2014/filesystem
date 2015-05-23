TOPDIR = ../../..
#TESTDIR = ../test

include $(TOPDIR)/GlobalMakefile
object = namespace.o
target = $(TOPDIR)/lib/namespace.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^

default:: $(object)

test::
	cd $(TESTDIR) &&  make || exit 1;

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

