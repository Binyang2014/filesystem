TOPDIR = ..

include $(TOPDIR)/GlobalMakefile

SUBDIRS = client data-server master common data-master
 
include $(TOPDIR)/GlobalSubDirs

LIBDIR = $(TOPDIR)/lib
LIB = $(LIBDIR)/libcommon.a $(LIBDIR)/libvfs.a $(LIBDIR)/libdataserver.a $(LIBDIR)/libmachinerole.a $(LIBDIR)/libdatamaster.a $(LIBDIR)/libclient.a 

include $(TOPDIR)/GlobalMakefile
obj = main.o

system:: fileapp

fileapp: main.o $(LIB)
	$(CC) $(CFLAGS) $^ -L$(LIBDIR) -lcommon -lvfs -lclient -ldataserver -ldatamaster -lmachinerole -o $@ -lpthread -lm

include $(TOPDIR)/GlobalRules

include $(obj:.$(OBJSUF)=.d)
