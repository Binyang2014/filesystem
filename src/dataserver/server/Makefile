TOPDIR = ../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libtool.a $(LIBDIR)/libvfs.a
TEST_PROGRAM = dataserver_test buffer_test

include $(TOPDIR)/GlobalMakefile

objects = dataserver_test.o dataserver.o dataserver_comm_handler.o dataserver_buff.o dataserver_operations.o
buffer_test_obj = buffer_test.o dataserver_buff.o

default:: $(objects)

include $(TOPDIR)/GlobalRules
include $(objects:.o=.d)

dataserver_test:$(objects) $(LIB)
	$(CC) $(CFLAGS) $(objects) -L$(LIBDIR) -lvfs -ltool -o $@

buffer_test: $(buffer_test_obj) $(LIB)
	$(CC) $(CFLAGS) $(buffer_test_obj) -L$(LIBDIR) -ltool -o $@
