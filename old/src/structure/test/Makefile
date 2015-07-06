TOPDIR = ../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libstruct.a
TEST_PROGRAM = bitmap_test basic_queue_test basic_list_test

include $(TOPDIR)/GlobalMakefile
test_obj = bitmap_test.o basic_queue_test.o basic_list_test.o

all_test:: $(TEST_PROGRAM)

bitmap_test: bitmap_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lstruct -o $@

basic_queue_test: basic_queue_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lstruct -o $@

basic_list_test: basic_list_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lstruct -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
