/*
 * client.c
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */

/**
 * 1. 监听来自客户的请求
 */
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <mpi.h>
#include <assert.h>
#include "client.h"
#include "../structure/basic_queue.h"
#include "../tool/message.h"
#include "../tool/file_tool.h"
#include "../global.h"
#include "../tool/errinfo.h"
#include "../structure/basic_list.h"

//TODO temporary definition
int master_rank = 0;

/*================private variables===============*/
static char *send_buf;
static char *receive_buf;
static char *file_buf;
static basic_queue_t *message_queue;
static char *create_file_buff;

/*====================private declarations====================*/
static void send_data(char *file_name, unsigned long file_size, list_t *list);
static int client_create_file_op(char *file_path, char *file_name);

/*====================private functions====================*/
static void send_data(char *file_name, unsigned long file_size, list_t *list)
{
	FILE *fp = fopen(file_name, "r");
	int block_num = ceil((double) file_size / MAX_COUNT_DATA);
	int i = 0, j = 0, k = 0, read_size;
	list_iter_t *iter = list->list_ops->list_get_iterator(list, AL_START_HEAD);
	ans_client_create_file *ans;
	msg_w_ctod_t writer;
	writer.operation_code = 1;
	writer.offset = 0;

	msg_acc_candd_t acc_msg;
	acc_msg.operation_code = MSG_ACC;
	acc_msg.status = 0;

	msg_data_t data_msg;

	int result;
	MPI_Status status;
	while (i < block_num)
	{
		//printf("******i =  %d\n*****", i);
		ans = (ans_client_create_file *)list->list_ops->list_next(iter)->value;
		//printf("-------ans_size=====%d\n", ans->block_num);
		int cur_machine_id;
		int send_size;
		for(k = 0; k < ans->block_num;){
			send_size = 0;
			cur_machine_id = (ans->block_global_num + k) ->server_id;
			while(1){
				//sleep(3);
				if(k < ans->block_num && cur_machine_id == (ans->block_global_num + k)->server_id && send_size < MAX_COUNT_CID_W){
					//printf("cur_machine_id %d %d %d %d %d %d\n", cur_machine_id, (ans->block_global_num + k)->server_id, send_size, MAX_COUNT_CID_W, k, ans->block_num);
					writer.chunks_id_arr[send_size++] = (ans->block_global_num + k)->global_id;
					k++;
				}else{
					//printf("else cur_machine_id %d %d %d %d %d %d %d\n", cur_machine_id, ans->block_global_num[k].server_id, send_size,MAX_COUNT_CID_W, k, ans->block_num, i);
					break;
				}
			}

			writer.chunks_count = send_size;
			MPI_Send(&writer, sizeof(msg_w_ctod_t), MPI_CHAR, cur_machine_id, D_MSG_CMD_TAG, MPI_COMM_WORLD);
			MPI_Send(&acc_msg, MAX_CMD_MSG_LEN, MPI_CHAR, cur_machine_id, 13, MPI_COMM_WORLD);
			//MPI_Recv(&result, 1, MPI_INT, cur_machine_id, 1, MPI_COMM_WORLD, &status);
			//writer.
			for(j = 0; j < send_size; j++){
				assert(i < block_num);
				if (i != block_num - 1){
					read_size = fread(file_buf, sizeof(char), MAX_COUNT_DATA, fp);
					fseek(fp, MAX_COUNT_DATA, MAX_COUNT_DATA * i);
					data_msg.tail = 1;
				} else{
					read_size = fread(file_buf, sizeof(char), file_size - (block_num - 1) * MAX_COUNT_DATA, fp);
					data_msg.tail = 0;
				}
//				int ind;
//				for(ind = 0; ind < read_size; ind++){
//					putchar(file_buf[ind]);
//				}
				data_msg.offset = 0;
				data_msg.seqno = 0;
				data_msg.len = read_size;
				memcpy(data_msg.data, file_buf, MAX_COUNT_DATA);
				puts("START SEND DATA");
				MPI_Send(&data_msg, MAX_DATA_MSG_LEN, MPI_CHAR, cur_machine_id, 13, MPI_COMM_WORLD);
				//printf("i = %d send_size = %d block_num = %d\n", i, send_size, block_num);
				i++;
			}
		}
	}
	puts("FINIST SEND DATA");
	fclose(fp);
}

static inline ans_client_create_file *alloc_create_file_buff(char *buff, int length){
	ans_client_create_file *ans = (ans_client_create_file *)malloc(sizeof(ans_client_create_file));
	memcpy(ans, buff, length);
	return ans;
}

static int client_create_file_op(char *file_path, char *file_name)
{
	long file_length = file_size(file_path);
	if (file_length == -1)
	{
		return -1;
	}

	int result;
	int master_malloc_result;

	MPI_Status status;

	client_create_file message;
	message.operation_code = CREATE_FILE_CODE;
	message.file_size = file_length;
	strcpy(message.file_name, file_name);
	memcpy(send_buf, &message, sizeof(client_create_file));

	//TODO not stable
	MPI_Send(send_buf, MAX_CMD_MSG_LEN, MPI_CHAR, master_rank, CLIENT_INSTRCTION_MESSAGE_TAG, MPI_COMM_WORLD);
	err_ret("client.c: client_create_file_op waiting for allocate answer");
	MPI_Recv(&master_malloc_result, 1, MPI_INT, master_rank, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD, &status);
	if(master_malloc_result == 0){
		err_ret("client.c: client_create_file_op master allocate new file space failed");
		return -1;
	}
	ans_client_create_file *ans;
	err_ret("client.c: client_create_file_op waiting for allocate location");
	if(master_malloc_result == 1)
	{
		err_ret("client.c: client_create_file_op allocate location success ");
		list_t *list = list_create();
		list->free = free;
		do
		{
			MPI_Recv(create_file_buff, sizeof(ans_client_create_file), MPI_CHAR, master_rank, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD, &status);
			ans = (ans_client_create_file *)create_file_buff;
			list->list_ops->list_add_node_tail(list, alloc_create_file_buff(create_file_buff, sizeof(ans_client_create_file)));
			int i;
			//for(i = 0; i != ans->block_num; i++)
			//{
			//	printf(" server_id == %d\n", ans->block_num);
			//}
		}while(!ans->is_tail);

//		list_iter_t *iter = list->list_ops->list_get_iterator(list, AL_START_HEAD);
//		ans = (ans_client_create_file *)list->list_ops->list_next(iter);
//		while(ans != NULL){
//			printf(" server_id == %d\n", ans->block_num);
//			ans = (ans_client_create_file *)(list->list_ops->list_next(iter)->value);
//		}

		send_data(file_path, file_length, list);
		list_release(list);
	}
	//TODO 如果master成功分配内存，那么此时应该等待接收data server机器信息
	//log_info("client_create_file end");
	err_ret("client.c: client_create_file_op end without error");
	return 0;
}

void *client_init(void *arg) {
	send_buf = (char*) malloc(MAX_CMD_MSG_LEN);
	receive_buf = (char*) malloc(MAX_CMD_MSG_LEN);
	file_buf = (char *)malloc(MAX_COUNT_DATA);
	create_file_buff = (char *)malloc(sizeof(ans_client_create_file));
	message_queue = alloc_basic_queue(sizeof(common_msg_t), -1);
	message_queue->dup = common_msg_dup;
	//message_queue->free = common_msg_free;

	if(send_buf == NULL || receive_buf == NULL || file_buf == NULL || message_queue == NULL){
		client_destroy();
	}

	//puts("********************hehehehe********************");
	client_create_file_op("/home/ron/test/read.in", "/readin");
	//puts("hehehehe");
	//client_create_file_op("/home/ron/test/read.in", "/readin");
	//client_create_file_op("/home/ron/test/read.in", "/readin");
	err_ret("end create file");
	return 0;
}

int client_destroy(){
	free(send_buf);
	free(receive_buf);
	free(file_buf);
	destroy_basic_queue(message_queue);
}




