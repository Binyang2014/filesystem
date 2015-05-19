#src/main
cc=mpicc
objects=master_data_servers.o namespace.o master.o message.o
filesystem: ${objects}
master_data_servers.o: ./data_server/master_data_servers.c ./data_server/master_data_servers.h ../tool/message.h
	${cc} -c ./data_server/master_data_servers.c
namespace.o: ./namespace/namespace.c ./namespace/namespace.h ../structure/namespace_code.h\
			../tool/hash.h ../structure/basic_queue.h
	${cc} -c ./namespace/namespace.c
master.o: master.c master.h ./namespace/namespace.h ./data_server/master_data_servers.h ../structure/basic_queue.h\
			../structure/namespace_code.h ../tool/message.h ../global.h
	${cc} -c master.c

#src/structure
message.o: ../tool/message.c ../tool/message.h ../global.h
	${cc} -c ../tool/message.c
	
#src/tool

#src/client

#src/dataserver


clean:
	rm ${objects}