TOPDIR = ../..
TESTDIR = ./test

include $(TOPDIR)/GlobalMakefile
object = basic_queue.o bitmap.o basic_list.o namespace_code.o data_server_location.o
target = $(TOPDIR)/lib/libstruct.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^

default:: $(object)

test::
	cd $(TESTDIR) &&  make || exit 1;

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

