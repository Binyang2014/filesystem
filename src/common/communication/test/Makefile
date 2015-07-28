TOPDIR = ../../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libcommon.a 
TEST_PROGRAM = mpi_communication_test rpc_test

include $(TOPDIR)/GlobalMakefile
test_obj = mpi_communication_test.o rpc_test.o

all_test:: $(TEST_PROGRAM)

mpi_communication_test: mpi_communication_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

rpc_test: rpc_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
