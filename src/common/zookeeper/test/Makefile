TOPDIR = ../../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libcommon.a
TEST_PROGRAM = ztree_test zserver_test zookeeper_test

include $(TOPDIR)/GlobalMakefile
test_obj = ztree_test.o zserver_test.o zookeeper_test.o

all_test:: $(TEST_PROGRAM)

ztree_test: ztree_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

zserver_test: zserver_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

zookeeper_test: zookeeper_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
