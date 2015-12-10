TOPDIR = ../../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libcommon.a
TEST_PROGRAM = shmem_server_test shmem_client_test fifo_server_test fifo_client_test msg_test

include $(TOPDIR)/GlobalMakefile
test_obj = shmem_server_test.o shmem_client_test.o fifo_server_test.o fifo_client_test.o msg_test.o

all_test:: $(TEST_PROGRAM)

shmem_server_test: shmem_server_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

shmem_client_test: shmem_client_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

fifo_server_test: fifo_server_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

fifo_client_test: fifo_client_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

msg_test: msg_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
