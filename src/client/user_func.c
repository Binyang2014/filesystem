#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi_communication.h"
#include "client.h"
#include "log.h"
#include "user_func.h"
#include "time_tool.h"
#include "test_conf.h"

static int block_size_type[] = {1 << 10, 1 << 11, 1 << 12, 1 << 13,
		1 << 14, 1 << 15, 1 << 16, 1 << 17, 1 << 18,
		1 << 19, 1<< 20, 1 << 21, 1 << 22, 1 << 23};
static int file_size_type[] = {1 << 27, 1 << 28, 1 << 29, 1 << 30};

static void fill_buff(char *buff, uint64_t buff_size, int rank)
{
	int index = 0;
	while(index < buff_size)
	{
		buff[index++] = (rand() + rank % 26) + 'A';
	}
}


static char *generate_name(int rank)
{
	char *file_name = malloc(200);
	fill_buff(file_name, 30, rank);
	file_name[30] = 0;
	return file_name;
}

static void get_file_name(char *result, char *name, uint64_t block_size, uint64_t file_size)
{
	block_size = (block_size >> 10);
	file_size = (file_size >> 20);
	if(block_size < (1 << 10))
	{
		sprintf(result, "%s/%dk_%"PRIu64"M", name, block_size, file_size);
	}else
	{
		block_size = (block_size >> 10);
		sprintf(result, "%s/%dM_%"PRIu64"M", name, block_size, file_size);
	}
}

/**
 * select rank to execute as client
 * try best to make a balance, it's a ideal situation
 */
static int select_execute_process(int rank, int client_num)
{
	if(rank == 0)
	{
		return 0;
	}
	int size = get_mpi_size();
	int node_num = size / NODE_PROCESS_NUM;
	int rank_node_seq = rank / NODE_PROCESS_NUM;
	int node_execute_process = client_num / NODE_PROCESS_NUM;
	int remain = client_num % node_num;
	int node_in_remain = rank % NODE_PROCESS_NUM - 1;

	if(remain == 0)
	{
		if(rank_node_seq == 0)
		{
			return node_in_remain <= node_execute_process;
		}
		else
		{
			return node_in_remain < node_execute_process;
		}
	}
	else
	{
		if(node_in_remain < node_execute_process)
		{
			return 1;
		}

		if(rank_node_seq < remain)
		{
			return (node_in_remain <= node_execute_process) ||
					(rank_node_seq == 0 && node_in_remain <= node_execute_process + 1 && node_execute_process < NODE_PROCESS_NUM - 1);
		}
		return 0;
	}
}

static void file_append(int fd, char *buff, size_t size, uint64_t times)
{
	while(times-- > 0)
	{
		f_append(fd, buff, size);
	}
}

static void file_speed_test_help(uint64_t block_size, uint64_t file_size, int rank)
{
	char file_name[100];
	char *home = getenv("HOME");
	get_file_name(file_name, home, block_size, file_size);
	uint64_t times = EXECUTE_TIMES * (file_size / block_size);
	char *buff = malloc(block_size);
	fill_buff(buff, block_size, rank);
	int fd = f_open(file_name, CREATE_TEMP | RDWR, RUSR | WUSR);
	timeval_t *start = get_timestamp();
	file_append(fd, buff, block_size, times);
	timeval_t *end = get_timestamp();
	printf("file speed test end, file name is %s, time cost is %d\n", file_name, cal_time(end, start));
	f_close(fd);
}

static void file_speed_test(int rank)
{
	timeval_t *start = get_timestamp();
	int i = 0;
	int j = 0;
	for(i = 0; i < FILE_NUM; i++)
	{
		for(j = 0; j < BLOCK_NUM; j++)
		{
			file_speed_test_help(block_size_type[j],  file_size_type[i], rank);
		}
	}
	timeval_t *end = get_timestamp();
	printf("file speed test end and time cost is %d\n", cal_time(end, start));
}

//static void huge_file_test(uint64_t block_num, uint64_t file_size)
//{
//	char file_name[1024];
//	get_file_name(file_name, "/home/buaajsi/huge_file_test/", block_num, file_size);
//	int fd = f_open(file_name, CREATE_TEMP | RDWR, RUSR | WUSR);
//	uint64_t times = EXECUTE_TIMES * (file_size / block_num);
//	char *buff = malloc(block_num);
//	fill_buff(buff, block_num);
//	timeval_t *t_start = get_timestamp();
//	file_append(fd, buff, block_num, times);
//	timeval_t *t_end = get_timestamp();
//	printf("huge file test end, file name is %s, time cost is %d\n", file_name, cal_time(t_end, t_start));
//	f_close(fd);
//}

static void multi_client_same_file(int rank, char *name, int block_size, int times)
{
	int fd = f_open(name, CREATE_TEMP | RDWR, RUSR | WUSR);
	char *buff = malloc(block_size);
	fill_buff(buff, block_size, rank);
	file_append(fd, buff, block_size, times);
}

static void multi_client_diff_file(int rank, int block_size, int times)
{
	char *file_name = generate_name(rank);
	int fd = f_open(file_name, CREATE_TEMP | RDWR, RUSR | WUSR);
	char *buff = malloc(block_size);
	fill_buff(buff, block_size, rank);
	file_append(fd, buff, block_size, times);
}

void system_test(int rank)
{
	srand((unsigned) (time(NULL)));
#ifdef WRITE_SPEED_TEST
	puts("WRITE_SPEED_TEST");
	int size = get_mpi_size();
	if(rank == size - 1)
	{
		timeval_t *start = get_timestamp();
		file_speed_test(rank);
		timeval_t *end = get_timestamp();
		printf("file speed test end, total time cost is %d\n", cal_time(end, start));
	}
#endif

#ifdef HUGE_FILE_TEST
	puts("HUGE_FILE_TEST");
	int size = get_mpi_size();
	if(rank == size - 1)
	{
		timeval_t *start = get_timestamp();
		huge_file_test(1 << 10, 10);
		timeval_t *end = get_timestamp();
		printf("huge file test end, total time cost is %d\n", cal_time(end, start));
	}
#endif

#ifdef MULTI_SAME_TEST_BLOCK
	printf("MULTI_CLIENT_SAME_FILE_TEST %d\n", MULTI_SAME_TEST_BLOCK);
	if(select_execute_process(rank, MULTI_SAME_CLIENT_NUM))
	{
		char *file_name = MULTI_SAME_FILE_NAME;
		timeval_t *t_start = get_timestamp();
		multi_client_same_file(rank, file_name, MULTI_SAME_TEST_BLOCK, MULTI_SAME_TEST_FILE_SIZE / (MULTI_SAME_TEST_BLOCK * MULTI_SAME_CLIENT_NUM));
		timeval_t *t_end = get_timestamp();
		printf("multi client same file test end;file name is %s;time cost is %d;block size is %d;\n", file_name, cal_time(t_end, t_start), MULTI_SAME_TEST_BLOCK);
	}
#endif

#ifdef MULTI_DIFF_TEST_BLOCK
	printf("MULTI_CLIENT_DIFF_FILE_TEST %d\n", MULTI_DIFF_TEST_BLOCK);
	if(select_execute_process(rank, MULTI_DIFF_CLIENT_NUM))
	{
		timeval_t *t_start = get_timestamp();
		multi_client_diff_file(rank, MULTI_DIFF_TEST_BLOCK, MULTI_DIFF_TEST_FILE_SIZE / (MULTI_DIFF_TEST_BLOCK * MULTI_DIFF_CLIENT_NUM));
		timeval_t *t_end = get_timestamp();
		printf("multi client diff file test end;time cost is %d;block size is %d;\n", cal_time(t_end, t_start), MULTI_DIFF_TEST_BLOCK);
	}
#endif
}

void user_func()
{
	int fd;
	char buf[] = "1 2 3 4 5 6 7 8 9 10";
	char read_buf[30] = {0};

	printf("-------------hello--------------\n");
	fd = f_open("/temp/a.txt", CREATE_TEMP | RDWR, RUSR | WUSR);
	if(fd < 0)
	{
		log_write(LOG_ERR, "open file failed");
		exit(1);
	}
	fd = f_open("/temp/a.txt", CREATE_TEMP | RDWR, RUSR | WUSR);
	log_write(LOG_DEBUG, "file open successfully");
	f_close(fd);
	f_open("/temp/a.txt", RDWR);
	f_append(fd, buf, sizeof(buf));
	memset(buf, 0, 21);
	f_read(fd, read_buf, 21);
	printf("1data is %s\n", read_buf);
	memset(buf, 0, 21);

	f_read(fd, read_buf, 21);
	printf("2data is %s\n", read_buf);
	memset(buf, 0, 21);
	f_read(fd, read_buf, 21);
	printf("3data is %s\n", read_buf);
	memset(buf, 0, 21);

	f_read(fd, read_buf, 21);
	printf("4data is %s\n", read_buf);

	f_close(fd);

	//read again
	f_open("/temp/a.txt", RDWR);
	f_read(fd, read_buf, 21);
	printf("data is %s\n", read_buf);
	f_close(fd);
}
