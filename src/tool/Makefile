object = bitmap_test.o bitmap.o
bitmap_test: $(object)
	cc -g -o bitmap_test $(object)
bitmap.o:bitmap.c
	cc -g -c bitmap.c
bitmap_test.o:bitmap_test.c
	cc -g -c bitmap_test.c
.PHONY:clean
clean:
	rm -f *.o  bitmap_test
