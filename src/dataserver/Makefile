objects = test.o dataserver.o

test:$(objects)
	cc -o test $(objects)
dataserver.o:dataserver.h dataserver.c
	cc -c -g dataserver.c

.PHONY:clean
clean:
	rm test $(objects)
