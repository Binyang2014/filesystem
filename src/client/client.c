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

typedef struct send_data_block{
	unsigned int write_len;
	unsigned int offset;
	unsigned long global_id;
}send_data_block_t;

/*================private variables===============*/
static char *send_buf;
static char *receive_buf;
static char *file_buf;
static basic_queue_t *message_queue;
static char *create_file_buff;

/*====================private declarations====================*/
static void send_data(char *file_name, unsigned long file_size, list_t *list);
static int client_create_file_op(char *file_path, char *file_name);
//static int client_read_file_op(char *file_path, char *file_name);

/*====================private functions====================*/
static void send_data(char *file_name, unsigned long file_size, list_t *list)
{
	FILE *fp = fopen(file_name, "r");
	int block_send_size = ceil((double) BLOCK_SIZE / MAX_DATA_CONTENT_LEN);

	int j = 0, k = 0, read_size;
	unsigned long write_offset = 0;
	list_iter_t *iter = list->list_ops->list_get_iterator(list, AL_START_HEAD);
	ans_client_create_file *ans;

	basic_queue_t *block_queue = (basic_queue_t *)alloc_basic_queue(sizeof(send_data_block_t), MAX_COUNT_CID_W);

	send_data_block_t send_block;

	msg_w_ctod_t writer;
	writer.operation_code = 0x01;
	writer.offset = 0;
	writer.write_len = 0;
	writer.unique_tag = 13;

	msg_acc_candd_t *acc_msg;
	msg_data_t data_msg;

	MPI_Status status;

	/*queue_len should be the */

	puts("START SEND DATA");
	while (write_offset < file_size)
	{
		ans = (ans_client_create_file *)list->list_ops->list_next(iter)->value;
		int cur_machine_id;
		//int block_send_size;
		for(k = 0; k < ans->block_num;){
			cur_machine_id = (ans->block_global_num + k) ->server_id;
			printf("server_id = %d\n", k);
			writer.write_len = 0;
			printf("*****send_size = %d*****\n", block_queue->current_size);
			while(k < ans->block_num && cur_machine_id == (ans->block_global_num + k)->server_id && block_queue->current_size + block_send_size <= MAX_COUNT_CID_W){
				//printf("*****send_size = %d*****\n", block_queue->current_size);
				send_block.offset = 0;
				while(send_block.offset < (ans->block_global_num + k)->write_len){
					send_block.global_id = (ans->block_global_num + k)->global_id;
					if(send_block.offset + MAX_DATA_CONTENT_LEN < (ans->block_global_num + k)->write_len){
						send_block.write_len = MAX_DATA_CONTENT_LEN;
						send_block.offset += MAX_DATA_CONTENT_LEN;
					}else{
						send_block.write_len = (ans->block_global_num + k)->write_len - send_block.offset;
						send_block.offset =  (ans->block_global_num + k)->write_len;
					}
					writer.write_len += send_block.write_len;
					block_queue->basic_queue_op->push(block_queue, &send_block);
				}
				k++;
			}

			writer.chunks_count = block_queue->current_size;
			int t;
			for(t = 0; t != block_queue->current_size; t++){
				writer.chunks_id_arr[t] = ((send_data_block_t *)get_queue_element(block_queue, t))->global_id;
				//printf("*****block_id = %d*****\n", writer.chunks_id_arr[t]);
			}
			printf("*****send_size = %d*****\n", block_queue->current_size);
			printf("cur_machine_id = %d", cur_machine_id);
			MPI_Send(&writer, MAX_CMD_MSG_LEN, MPI_CHAR, cur_machine_id, D_MSG_CMD_TAG, MPI_COMM_WORLD);
			//while(1);
			//printf("*****send_size = %d*****\n", block_queue->current_size);
//			MPI_Send(&acc_msg, MAX_CMD_MSG_LEN, MPI_CHAR, cur_machine_id, 13, MPI_COMM_WORLD)
			MPI_Recv(receive_buf, MAX_CMD_MSG_LEN, MPI_CHAR, cur_machine_id, 13, MPI_COMM_WORLD, &status);
			acc_msg = receive_buf;
			//printf("*****send_size = %d*****\n", block_queue->current_size);
			int tmp_write_offset = write_offset;
			for(j = 0; j < writer.chunks_count; j++){

				if (tmp_write_offset + MAX_DATA_CONTENT_LEN < file_size){
					read_size = fread(file_buf, sizeof(char), MAX_DATA_CONTENT_LEN, fp);
					fseek(fp, MAX_DATA_CONTENT_LEN, tmp_write_offset);
					tmp_write_offset += MAX_DATA_CONTENT_LEN;
				} else{
					read_size = fread(file_buf, sizeof(char), file_size - tmp_write_offset, fp);
					tmp_write_offset = file_size;
				}
				if(j == writer.chunks_count - 1){
					data_msg.tail = 1;
					printf("last message %d\n", data_msg.len);
				}else{
					data_msg.tail = 0;
				}

				block_queue->basic_queue_op->pop(block_queue, &send_block);
				data_msg.offset = send_block.offset;
				data_msg.seqno = send_block.global_id;
				data_msg.len = send_block.write_len;
				memcpy(data_msg.data, file_buf, MAX_DATA_CONTENT_LEN);

			//	puts("&&&&start put content");
//				int index;
//				char *h = data_msg.data;
//				for(index = 0; index != data_msg.len; index++)
//					putchar(h[index]);
		//		puts("&&&&end put content");
				printf("Start Send %d/%d Data\n", j + 1, writer.chunks_count);
				MPI_Send(&data_msg, MAX_DATA_MSG_LEN, MPI_CHAR, cur_machine_id, 13, MPI_COMM_WORLD);
				printf("End Send %d/%d Data\n", j + 1, writer.chunks_count);
			}
			puts("1 FINIST SEND DATA");
			while(1);
			write_offset += writer.write_len;
			basic_queue_reset(block_queue);
			return;
		}
	}


	destroy_basic_queue(block_queue);
	fclose(fp);
}

static void create_local_file(char *file_path, list_t *list){

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

	//int result;
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
			//int i;
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

//static int client_read_file_op(char *file_path, char *file_name){
//	client_read_file *c_r_f = (client_read_file *)malloc(sizeof(client_read_file));
//	strcpy(c_r_f->file_name, file_name);
//	c_r_f->file_size = 0;
//
//	MPI_Send(c_r_f, MAX_CMD_MSG_LEN, MPI_CHAR, master_rank, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD);
//
//	int location_exists;
//	MPI_Status status;
//	MPI_Recv(&location_exists, 1, MPI_INT, master_rank, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD, &status);
//	if(!location_exists){
//		err_ret("FILE NOT EXISTS");
//		return 0;
//	}else{
//		ans_client_read_file *ans;
//		list_t *list = list_create();
//		list->free = free;
//		do{
//			MPI_Recv(create_file_buff, sizeof(ans_client_read_file), MPI_CHAR, master_rank, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD, &status);
//			ans = (ans_client_read_file *)create_file_buff;
//			list->list_ops->list_add_node_tail(list, alloc_create_file_buff(create_file_buff, sizeof(ans_client_read_file)));
//		//	int i;
//			//for(i = 0; i != ans->block_num; i++)
//			//{
//			//	printf(" server_id == %d\n", ans->block_num);
//			//}
//		}while(!ans->is_tail);
//	}
//	return 0;
//}

void *client_init(void *arg) {
	send_buf = (char*) malloc(MAX_CMD_MSG_LEN);
	receive_buf = (char*) malloc(MAX_CMD_MSG_LEN);
	file_buf = (char *)malloc(MAX_CMD_MSG_LEN);
	create_file_buff = (char *)malloc(sizeof(ans_client_create_file));
	message_queue = alloc_basic_queue(sizeof(common_msg_t), -1);
	message_queue->dup = common_msg_dup;
	//message_queue->free = common_msg_free;

	if(send_buf == NULL || receive_buf == NULL || file_buf == NULL || message_queue == NULL){
		client_destroy();
	}

	//puts("********************hehehehe********************");
	//client_create_file_op("/home/ron/test/readfile.cvs", "/readin");
	//puts("hehehehe");
	client_create_file_op("/home/binyang/Test/test", "/readin");
	//client_create_file_op("/home/ron/test/read.in", "/readin");
	err_ret("end create file");

	//client_
	return 0;
}

int client_destroy(){
	free(send_buf);
	free(receive_buf);
	free(file_buf);
	destroy_basic_queue(message_queue);
}




