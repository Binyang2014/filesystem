TOPDIR = ../../..
TESTDIR = ./test

include $(TOPDIR)/GlobalMakefile
object = dataserver_buff.o
target = $(TOPDIR)/lib/libdataserver.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^

default:: $(object)

test::
	cd $(TESTDIR) &&  make || exit 1;

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

