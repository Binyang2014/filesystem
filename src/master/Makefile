TOPDIR = ../..

include $(TOPDIR)/GlobalMakefile
object = master.o namespace.o master_data_servers.o
target = $(TOPDIR)/lib/libmaster.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^
	
default:: $(object)

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)