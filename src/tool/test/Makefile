TOPDIR = ../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libtool.a
TEST_PROGRAM = bitmap_test err_test hash_test

include $(TOPDIR)/GlobalMakefile
test_obj = bitmap_test.o err_test.o hash_test.o

all_test:: $(TEST_PROGRAM)

bitmap_test: bitmap_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -ltool -o $@

err_test: err_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -ltool -o $@

hash_test: hash_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -ltool -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
