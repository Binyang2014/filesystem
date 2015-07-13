TOPDIR = ../..
TESTDIR = ./test

include $(TOPDIR)/GlobalMakefile
object = log.o zmalloc.o mpi_communication.o sds.o basic_queue.o basic_list.o syn_tool.o threadpool.o
target = $(TOPDIR)/lib/libcommon.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^

default:: $(object)

test::
	cd $(TESTDIR) &&  make || exit 1;

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

