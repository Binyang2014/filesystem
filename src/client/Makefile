TOPDIR = ../..
TESTDIR = ./test

include $(TOPDIR)/GlobalMakefile
object = client_struct.o client_server.o client.o user_func.o
target = $(TOPDIR)/lib/libclient.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^

default:: $(object)

test::
	cd $(TESTDIR) &&  make || exit 1;

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

