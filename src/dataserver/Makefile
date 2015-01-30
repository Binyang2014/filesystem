objects = test.o dataserver.o

test:$(objects)
	gcc -o test $(objects)
dataserver.o:dataserver.h
	gcc -c -g dataserver.c

.PHONY:clean
clean:
	rm test $(objects)
