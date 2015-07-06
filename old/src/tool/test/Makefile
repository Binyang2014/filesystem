TOPDIR = ../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libtool.a $(LIBDIR)/libstruct.a
TEST_PROGRAM = syn_tool_test err_test hash_test threadpool_test

include $(TOPDIR)/GlobalMakefile
test_obj = syn_tool_test.o err_test.o hash_test.o threadpool_test.o

all_test:: $(TEST_PROGRAM)

syn_tool_test: syn_tool_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -ltool -lstruct -o $@

err_test: err_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -ltool -o $@

hash_test: hash_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -ltool -o $@

threadpool_test: threadpool_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -ltool -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
