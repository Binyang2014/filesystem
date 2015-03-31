TOPDIR = ../../..
LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libtool.a
TEST_PROGRAM = struct_test vfs_test

include $(TOPDIR)/GlobalMakefile

objects = basic_structure.o  vfs_operations.o vfs_structure.o 
structure_test_obj = basic_structure.o structure_test.o 
vfs_test_obj = vfs_test.o basic_structure.o  vfs_operations.o vfs_structure.o 

default:: $(objects)

include $(TOPDIR)/GlobalRules
include $(objects:.$(OBJSUF)=.d)
include $(vfs_test_obj:.$(OBJSUF)=.d)

struct_test:$(structure_test_obj) $(LIB)
	$(CC) $(CFLAGS) $(structure_test_obj) -L$(LIBDIR) -ltool -o $@

vfs_test:$(vfs_test_obj) $(LIB)
	$(CC) $(CFLAGS) $(vfs_test_obj) -L$(LIBDIR) -ltool -o $@
