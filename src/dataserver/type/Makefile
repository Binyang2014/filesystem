object=basic_structure.o structure_test.o ../../tool/bitmap.o
test:$(object)
	cc -o test -g $(object)
structure_test.o:
	cc -g -c structure_test.c
basic_structure.o:../../tool/bitmap.h
	cc -g -c basic_structure.c
.PHONY:clean
clean:
	rm -f *.o test
