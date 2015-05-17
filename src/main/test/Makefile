namespace_test:
	gcc -o namespace_test namespace_test.c ../namespace/namespace.c ../../tool/hash.c ../../tool/errinfo.c
master_data_server_test:
	mpicc -o test_data_server master_data_server_test.c ../data_server/master_data_servers.c ../../structure/basic_queue.c -lm 

clean:
	rm namespace_test test_data_server


