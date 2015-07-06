TOPDIR = ../..
TESTDIR = ./test

include $(TOPDIR)/GlobalMakefile
object = hash.o errinfo.o message.o syn_tool.o threadpool.o file_tool.o
target = $(TOPDIR)/lib/libtool.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^

default:: $(object)

test::
	cd $(TESTDIR) &&  make || exit 1;

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

