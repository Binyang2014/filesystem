TOPDIR = ../../..
TESTDIR = ./test

include $(TOPDIR)/GlobalMakefile
object = message.o mpi_communication.o rpc_client.o rpc_server.o
target = $(TOPDIR)/lib/libcommon.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^

default:: $(object)

test::
	cd $(TESTDIR) &&  make || exit 1;

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

