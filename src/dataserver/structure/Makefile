TOPDIR = ../../..
TEST_PROGRAM = test vfs_test

include $(TOPDIR)/GlobalMakefile

objects = basic_structure.o  vfs_operations.o vfs_structure.o ../../tool/bitmap.o ../../tool/hash.o 
structure_test_obj = basic_structure.o structure_test.o ../../tool/bitmap.o 
vfs_test_obj = vfs_test.o basic_structure.o  vfs_operations.o vfs_structure.o ../../tool/bitmap.o ../../tool/hash.o 

default:: $(objects)

include $(TOPDIR)/GlobalRules
include $(objects:.$(OBJSUF)=.d)
include $(vfs_test_obj:.$(OBJSUF)=.d)

struct_test:$(structure_test_obj)
	cc -o $@ -g $^

vfs_test:$(vfs_test_obj)
	cc -o $@ -g $^
