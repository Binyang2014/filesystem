TOPDIR = ../..
TESTDIR = ./test

include $(TOPDIR)/GlobalMakefile
object = log.o
target = $(TOPDIR)/lib/libcommon.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^

default:: $(object)

test::
	cd $(TESTDIR) &&  make || exit 1;

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

