TOPDIR = ../../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libcommon.a 
TEST_PROGRAM = log_test threadpool_test map_test sds_test

include $(TOPDIR)/GlobalMakefile
test_obj = log_test.o threadpool_test.o map_test.o sds_test.o

all_test:: $(TEST_PROGRAM)

#syn_tool_test: syn_tool_test.o $(LIB)
#	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -ltool -lstruct -o $@

map_test: map_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

threadpool_test: threadpool_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

log_test: log_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

sds_test: sds_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
