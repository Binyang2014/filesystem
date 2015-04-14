TOPDIR = ../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libtool.a $(LIBDIR)/libvfs.a
TEST_PROGRAM = dataserver_test

include $(TOPDIR)/GlobalMakefile

objects = dataserver_test.o dataserver.o dataserver_comm_handler.o

default:: $(objects)

include $(TOPDIR)/GlobalRules
include $(objects:.o=.d)

dataserver_test:$(objects) $(LIB)
	$(CC) $(CFLAGS) $(objects) -L$(LIBDIR) -lvfs -ltool -o $@
