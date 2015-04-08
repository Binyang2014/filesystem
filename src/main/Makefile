main : /
	mpicc client.c master.c main.c ../tool/file_tool.c ../tool/error_log.c -o main -lm

clean :/
	rm main

run : /
	mpirun -np 2 ./main
