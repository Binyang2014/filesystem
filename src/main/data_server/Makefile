TOPDIR = ../../..
#TESTDIR = ../test

include $(TOPDIR)/GlobalMakefile
object = master_data_servers.o
target = $(TOPDIR)/lib/master_data_servers.a

$(target): $(object)
	$(AR) $(ARFLAG) $@ $^

default:: $(object)

#test::
#	cd $(TESTDIR) &&  make || exit 1;

include $(TOPDIR)/GlobalRules

include $(object:.$(OBJSUF)=.d)

