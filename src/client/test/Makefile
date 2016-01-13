TOPDIR = ../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libcommon.a $(LIBDIR)/libvfs.a $(LIBDIR)/libdataserver.a $(LIBDIR)/libclient.a
TEST_PROGRAM = client_server_test

include $(TOPDIR)/GlobalMakefile
test_obj = client_server_test.o

all_test:: $(TEST_PROGRAM)

client_server_test: client_server_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lclient -ldataserver -lvfs -lcommon -o $@ -lpthread -lm 

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
