TOPDIR = ../..

include $(TOPDIR)/GlobalMakefile
object = bitmap_test.o bitmap.o

DEPENDINCLUDE = bitmap.h

deault:: $(DEPENDINCLUDE)

bitmap_test: $(object)
	cc -g -o bitmap_test $(object)

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

clean::
	rm -f *.o  bitmap_test
