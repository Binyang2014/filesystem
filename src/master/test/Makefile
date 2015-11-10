TOPDIR = ../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libcommon.a $(LIBDIR)/libmachinerole.a
TEST_PROGRAM = machine_role_test

include $(TOPDIR)/GlobalMakefile
test_obj = machine_role_test.o

all_test:: $(TEST_PROGRAM)

machine_role_test: machine_role_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -lmachinerole -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
