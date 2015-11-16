TOPDIR = ../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libcommon.a $(LIBDIR)/libdatamaster.a
TEST_PROGRAM = data_master_test

include $(TOPDIR)/GlobalMakefile
test_obj = data_master_test.o

all_test:: $(TEST_PROGRAM)

machine_role_test: data_master_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -ldatamaster-o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
