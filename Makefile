INCLUDE = -I$(TOPDIR)/src -I$(TOPDIR)/src/common/structure_tool \
-I$(TOPDIR)/src/common/communication -I$(TOPDIR)/src/common/zookeeper \
-I$(TOPDIR)/src/common/ipc -I$(TOPDIR)/src/data-server/structure \
-I$(TOPDIR)/src/data-master -I$(TOPDIR)/src/master \
-I$(TOPDIR)/src/data-server/server -I$(TOPDIR)/src/client

ld = ./lib
depend_order=${ld}/libcommon.a ${ld}/libclient.a \
${ld}/libdataserver.a ${ld}/libdatamaster.a \
${ld}/libmachinerole.a ${ld}/libvfs.a

default::
	cd src;\
	make || exit 1;\
	
install::

clean::
	rm -rf filesystem;\
	rm -rf ./lib/*.a;\
	cd src;\
	make clean || exit 1;\
