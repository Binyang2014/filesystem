/*
 * message.h
 *
 *  Created on: 2015年7月13日
 *      Author: ron
 *      Modified: Binyang
 *
 * Because we do not know how compiler with orgnize message structure so we must be sure that
 * all message structure is aligned in 64 bits.
 */

#ifndef SRC_COMMON_COMMUNICATION_MESSAGE_H_
#define SRC_COMMON_COMMUNICATION_MESSAGE_H_
#include <mpi.h>
#include "../../global.h"

//length of some type of messages
#define COMMON_MSG_HEAD 4
#define COMMON_MSG_LEN (MAX_CMD_MSG_LEN + COMMON_MSG_HEAD)
#define DATA_MSG_HEAD_LEN 16
#define MAX_DATA_MSG_LEN (MAX_DATA_CONTENT_LEN + DATA_MSG_HEAD_LEN)

//caculate how many chunks ids can a meesage carries just for read and write
//messages
#define R_W_MSG_HEAD_LEN 24
#define MAX_COUNT_CID_R ((MAX_CMD_MSG_LEN - R_W_MSG_HEAD_LEN) / 8) //max length of chunks id array in read message
#define MAX_COUNT_CID_W ((MAX_CMD_MSG_LEN - R_W_MSG_HEAD_LEN) / 8) //max length of chunks id array in write message
#define MAX_COUNT_DATA  ((MAX_DATA_MSG_LEN - DATA_MSG_HEAD_LEN) / 8) //max count of data in one message package

//communicate tag different kinds of message need different tags
#define CLIENT_INSTRCTION_MESSAGE_TAG 400
#define CLIENT_INSTRUCTION_ANS_MESSAGE_TAG 401

//operation code
//Mater should deal with
#define MACHINE_REGISTER_TO_MASTER 1001
#define MASTER_CREATE_PERSISTENT_FILE 1002
#define MASTER_DELETE_SYSTEM_FILE 1003
#define SUB_MASTER_HEART_BEAT 1004
#define SUB_MASTER_ASK_GLOBAL_ID 1005
#define CONSISTENT_FILE_TO_DISK 1006
#define APPEND_FILE 1007

//Sub-Master should deal with
#define MACHINE_ROLE_GET_ROLE 2001
#define DATA_MASTER_HEART_BEAT_CODE 2002

//Data-Master should deal with
#define CREATE_FILE_CODE 3001
#define DATA_SERVER_HEART_BEAT_CODE 3002
#define READ_FILE_CODE 3003
#define WRITE_FILE_CODE 3004
#define APPEND_FILE_CODE 3005
//#define CREATE_FILE_ANS_CODE 3005

//Data-Server should deal with
#define C_D_WRITE_BLOCK_CODE 4001
#define C_D_READ_BLOCK_CODE 4002

#define MSG_COMM_TO_CMD(p_common_msg) ((int8_t*)(p_common_msg) + COMMON_MSG_HEAD)

enum machine_role{
	MASTER,
	SUB_MASTER,
	DATA_MASTER,
	DATA_SERVER
};


/*-------------------------ROLE ALLOCATOR----------------------*/
typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;//use to idetify specific message
	int source;
	int tag;
	char ip[20];
}machine_register_role_t;

typedef struct {
	int waiting_struction;
}machine_wait_role_t;

typedef struct {
	char ip[16];
	enum machine_role role;
	int data_master_rank;
	int sub_master_rank;
	int master_rank;
}machine_role_t;

/**
 * It's a common message used by message buffer
 */
typedef struct{
	int source;
	uint16_t operation_code;
	uint16_t transfer_version;
	int8_t rest[MAX_CMD_MSG_LEN - COMMON_MSG_HEAD];
}common_msg_t;

/*-------------------MASTER MESSAGE StRUCTURE------------------*/
#define FILE_NAME_MAX_LENGTH 255

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	int source;
	int tag;
	int8_t name[FILE_NAME_MAX_LENGTH + 1];
}master_create_file_t;

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	int result_code;
}master_create_file_ans_t;

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	int source;
	int tag;
	uint64_t append_size;
}master_append_file_t;

typedef struct {
	int result_code;
}master_append_file_ans_t;

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	int source;
	int tag;
	int8_t name[FILE_NAME_MAX_LENGTH + 1];
}master_deleter_system_file_t;

typedef struct {
	int result_code;
}master_deleter_system_file_ans_t;

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	int source;
	int tag;
	uint64_t id_num;
}sub_master_ask_global_id_t;


typedef struct {
	uint64_t start;
	uint64_t end;
}master_global_id_ans_t;


/*-------------------SUB_MASTER MESSAGE STRCTURE--------------------*/

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	int source;
	int tag;
	int8_t file_name[FILE_NAME_MAX_LENGTH + 1];
	uint64_t append_size;
}s_m_allocate_tmp_space_t;

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	int source;
	int tag;
	int8_t file_name[FILE_NAME_MAX_LENGTH + 1];
}s_m_create_temporary_file_t;

typedef struct {
	int result_code;
}s_m_create_temporary_file_ans_t;

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	int source;
	int tag;
	int8_t file_name[FILE_NAME_MAX_LENGTH + 1];
}s_m_delete_temporary_file_t;

typedef struct {
	int result_code;
}s_m_delete_temporary_file_ans_t;

/*-------------------SUB_MASTER MESSAGE STRCTURE END-----------------*/

/*-------------------DATA_MASTER MESSAGE STRUCTURE-------------------*/
/*
 * structure instruction of create file
 */
typedef struct client_create_file{
	uint16_t operation_code;
	uint16_t transfer_version;
	uint32_t reserved;
	uint64_t file_size;
	int8_t file_name[CLIENT_MASTER_MESSAGE_CONTENT_SIZE];
}client_create_file;

typedef struct client_read_file{
	uint16_t operation_code;
	uint16_t transfer_version;
	uint32_t reserved;
	uint64_t file_size;
	int8_t file_name[CLIENT_MASTER_MESSAGE_CONTENT_SIZE];
}client_read_file;

typedef struct answer_confirm{
	int result;
}answer_confirm_t;

typedef struct block_location{
	uint32_t server_id;
	uint32_t write_len;
	uint64_t global_id;
	uint64_t block_seq;
}block_location;

/*
 * data server send heart beat to master
 */
typedef struct d_server_heart_beat{
	uint16_t operation_code;
	uint16_t transfer_version;
	int id;
}d_server_heart_beat_t;

/**
 * client send write cmd request to data server
 */
typedef struct c_d_write_cmd{
	uint16_t operation_code;
	uint16_t transfer_version;
	int block_num;
}c_d_cmd_t;

typedef struct c_d_block_data{
	block_location block_info;
	int8_t data[BLOCK_SIZE];
}c_d_block_data_t;

/*-------------------DATA_MASTER MESSAGE STRUCTURE END----------------*/

/*-------------------DATA_SERVER MESSAGE STRUCTURE--------------------*/
/**
 *Following structure defined message structure between client and data 
 *server there are many things should be taken a attention.
 *
 *First:  client may be communicate with many users, and a user may be invoke many file options,
 *        such as open many files. client should maintain user's conditions, like how many files
 *        has opened so user can use RPC to communicate with client.
 *
 *Second: each operation to a certain file on client side will invoke a thread on data server side,
 *        that is to say for every read or write operation, data server will use a thread to handle
 *        it.
 *        data server will not maintain a connection will a client, so a thread will not be blocked
 *        if there are no new command be received.
 *
 *Third:  open and close operation will have no affection on data server, it only has affect on 
 *        client, and client should store right information for users.
 */

//read message from client to data server
typedef struct{
	//64 bits
	uint16_t operation_code;
	uint16_t transfer_version;
	int unique_tag;//client must promise to offer a special tag to data server
	
	uint32_t offset;//offset from the begin of the first chunk that should be read
	uint32_t reserved;

	uint32_t read_len;//the length will read, just send to one data server
	uint32_t chunks_count;//the numbers of chunks will be read

	uint64_t long chunks_id_arr[MAX_COUNT_CID_R];
}read_c_to_d_t;

//return accept message when you are ready to receive data message
//status is 0 if excuse successfully
typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	uint32_t status;

	uint32_t reserved;
}acc_d_and_c_t;

//read and write use this message
typedef struct{
	uint32_t len;//data len in this package
	uint32_t offset;//form the beginning chunks

	uint32_t seqno;//number of this read
	uint16_t transfer_version;
	int8_t tail;//if tail
	int8_t reserved[1];

	uint64_t long data[MAX_COUNT_DATA];
}msg_data_t;

//origin write operation, do not support insert
typedef struct{
	//64 bits
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t unique_tag;
	uint32_t offset;
	//64 bits
	uint32_t write_len;
	uint32_t chunks_count;
	//64 bits aligned
	uint64_t long chunks_id_arr[MAX_COUNT_CID_W];
}write_c_to_d_t;

//you can use this structure to insert text to a file
//if a message can not contain enough block_ids, you can send a serials messages
// and the seqno and tail is useful for this condition.
typedef struct{
	uint16_t operation_code;
	uint16_t unique_tag;

	uint32_t seqno;
	int8_t tail;
	//...
}write_c_to_d_ins_t;

//block struct
typedef struct block{
	uint64_t file_seq; //file split sequence
	uint64_t block_global_id;
}block_t;

/*-------------------DATA_SERVER MESSAGE STRUCTURE END----------------*/

void common_msg_dup(void *dest, void *source);

#endif /* SRC_COMMON_COMMUNICATION_MESSAGE_H_ */
