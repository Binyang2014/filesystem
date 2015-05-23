#src/main
cc=mpicc
objects=main.o master_data_servers.o namespace.o master.o message.o basic_queue.o basic_list.o hash.o\
		file_tool.o errinfo.o client.o syn_tool.o

filesystem: ${objects}
	${cc} -o filesystem ${objects} -lm -lpthread -g -O0
	
#src/main
main.o: main.c master.h ../client/client.h
	${cc} -c main.c
master_data_servers.o: ./data_server/master_data_servers.c ./data_server/master_data_servers.h ../tool/message.h
	${cc} -c ./data_server/master_data_servers.c
namespace.o: ./namespace/namespace.c ./namespace/namespace.h ../structure/namespace_code.h \
			../tool/hash.h ../structure/basic_queue.h ../tool/errinfo.h
	${cc} -c ./namespace/namespace.c
master.o: master.c master.h ./namespace/namespace.h ./data_server/master_data_servers.h ../structure/basic_queue.h \
			../structure/namespace_code.h ../tool/message.h ../global.h ../tool/syn_tool.h
	${cc} -c master.c

#src/structure
basic_queue.o: ../structure/basic_queue.c ../structure/basic_queue.h ../global.h
	${cc} -c ../structure/basic_queue.c
basic_list.o: ../structure/basic_list.c ../structure/basic_list.h 
	${cc} -c ../structure/basic_list.c
	
#src/tool
message.o: ../tool/message.c ../tool/message.h ../global.h
	${cc} -c ../tool/message.c
hash.o: ../tool/hash.c ../tool/hash.h
	${cc} -c ../tool/hash.c
file_tool.o: ../tool/file_tool.c ../tool/file_tool.h
	${cc} -c ../tool/file_tool.c
errinfo.o: ../tool/errinfo.c ../tool/errinfo.h
	${cc} -c ../tool/errinfo.c
syn_tool.o: ../structure/basic_queue.h ../tool/syn_tool.h ../tool/syn_tool.c
	${cc} -c ../tool/syn_tool.c
#src/client
client.o: ../client/client.c ../client/client.h ../tool/file_tool.h ../tool/message.h ../structure/basic_queue.h\
			../global.h ../tool/errinfo.h
	${cc} -c ../client/client.c
	
	
#src/dataserver

.PHONY : clean
clean:
	-rm filesystem ${objects}