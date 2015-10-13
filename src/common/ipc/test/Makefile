TOPDIR = ../../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libcommon.a
TEST_PROGRAM = shmem_server_test shmem_client_test

include $(TOPDIR)/GlobalMakefile
test_obj = shmem_server_test.o shmem_client_test.o

all_test:: $(TEST_PROGRAM)

shmem_server_test: shmem_server_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

shmem_client_test: shmem_client_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
