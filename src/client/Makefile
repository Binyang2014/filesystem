TOPDIR = ../..

include $(TOPDIR)/GlobalMakefile
object = client.o
target = $(TOPDIR)/lib/client.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^
	
default:: $(object)

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)
