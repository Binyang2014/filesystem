TOPDIR = ../../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libcommon.a $(LIBDIR)/libvfs.a $(LIBDIR)/libdataserver.a
TEST_PROGRAM = dataserver_buff_test dataserver_test

include $(TOPDIR)/GlobalMakefile
test_obj = dataserver_buff_test.o

all_test:: $(TEST_PROGRAM)

dataserver_buff_test: dataserver_buff_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -ldataserver -lvfs -lcommon -o $@

dataserver_test: dataserver_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -ldataserver -lvfs -lcommon -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
