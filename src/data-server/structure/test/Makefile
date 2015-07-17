TOPDIR = ../../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libvfs.a 
TEST_PROGRAM = structure_test vfs_test

include $(TOPDIR)/GlobalMakefile
test_obj = structure_test.o vfs_test.o

all_test:: $(TEST_PROGRAM)

structure_test: structure_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lvfs -lcommon -o $@

vfs_test: vfs_test.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lvfs -lcommon -o $@

include $(TOPDIR)/GlobalRules

include $(test_obj:.$(OBJSUF)=.d)
