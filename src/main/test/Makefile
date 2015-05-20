cc=mpicc
parend_dir = ../../

namespace: namespace_test.c ../namespace/namespace.c\
		 ../../tool/hash.c ../../tool/errinfo.c
	cc -o namespace namespace_test.c ../namespace/namespace.c ../../tool/hash.c ../../tool/errinfo.c

server:
	cc -o server master_data_server_test.c ../data_server/master_data_servers.c ../../structure/basic_queue.c -lm 

clean:
	rm namespace server


