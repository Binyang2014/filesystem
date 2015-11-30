/*
 * message.h
 *
 *  Created on: 2015年7月13日
 *      Author: ron
 *      Modified: Binyang
 *
 * Because we do not know how compiler with orgnize message structure so we 
 * must be sure that all message structure is aligned in 64 bits. In CMD type
 * messges, we must offer tag to idetify witch client send it. We use tag to
 * identify different clients and transfer number to identify make true client
 * and server use same protocal, and this field is not used yet.
 */

#ifndef SRC_COMMON_COMMUNICATION_MESSAGE_H_
#define SRC_COMMON_COMMUNICATION_MESSAGE_H_
#include <stdint.h>
#include "global.h"

//some define about message source(ip address) and message tag(port number)
#define ANY_SOURCE -1
#define ANY_TAG -1
#define CMD_TAG 0
#define WATCH_TAG 500
#define IGNORE_LENGTH -1

//length of some type of messages
#define COMMON_MSG_HEAD 4
#define COMMON_MSG_LEN (MAX_CMD_MSG_LEN + COMMON_MSG_HEAD)
#define DATA_MSG_HEAD_LEN 16
#define MAX_DATA_MSG_LEN (MAX_DATA_CONTENT_LEN + DATA_MSG_HEAD_LEN)
#define DATA_SERVER_NUM 16
#define MAX_COUNT_CID_RET MAX_COUNT_CID_R

//caculate how many chunks ids can a meesage carries just for read and write
//messages
#define R_W_MSG_HEAD_LEN 24
#define MAX_COUNT_CID_R ((MAX_CMD_MSG_LEN - R_W_MSG_HEAD_LEN) / 8) //max length of chunks id array in read message
#define MAX_COUNT_CID_W ((MAX_CMD_MSG_LEN - R_W_MSG_HEAD_LEN) / 8) //max length of chunks id array in write message
#define MAX_COUNT_DATA  ((MAX_DATA_MSG_LEN - DATA_MSG_HEAD_LEN) / 8) //max count of data in one message package

//communicate tag different kinds of message need different tags
#define CLIENT_INSTRCTION_MESSAGE_TAG 400
#define CLIENT_INSTRUCTION_ANS_MESSAGE_TAG 401
#define CLIENT_LISTEN_TAG 402

//operation code
//ACC CODE
#define ACCEPT_REPLY 7007
#define ACC_OK 7001
#define ACC_FAIL 7002
#define READ_FAIL 7003
#define WRITE_FAIL 7004
#define INIT_WRITE_FAIL 7005
#define FILE_NOT_EXIST 7006
#define ACC_IGNORE 7111

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
#define REGISTER_TO_DATA_MASTER_CODE 3000
#define CREATE_TEMP_FILE_CODE 3001
#define CREATE_PERSIST_FILE_CODE 3002
#define DATA_SERVER_HEART_BEAT_CODE 3003
#define READ_FILE_CODE 3004
#define APPEND_FILE_CODE 3005
#define DELETE_TMP_FILE_CODE 3006
#define APPEND_TEMP_FILE_CODE 3007
#define READ_TEMP_FILE_CODE 3008

//#define CREATE_FILE_ANS_CODE 3005

//Data-Server should deal with
#define C_D_WRITE_BLOCK_CODE 4001
#define C_D_READ_BLOCK_CODE 4002

//Zookeeper message code
#define ZOO_PATH_MAX_LEN 256
#define ZOO_MAX_DATA_LEN 128
#define ZOO_CREATE_CODE 5001
#define ZOO_CREATE_PARENT_CODE 5002
#define ZOO_GET_CODE 5003
#define ZOO_GET_CHILDREN_CODE 5004
#define ZOO_SET_CODE 5005
#define ZOO_EXISTS_CODE 5006
#define ZOO_DELETE_CODE 5007

//Server stop code
//every handler should achieve this function
#define SERVER_STOP 9000

#define MSG_COMM_TO_CMD(p_common_msg) ((int8_t*)(p_common_msg) + COMMON_MSG_HEAD)

//reply message type and it not complete
typedef enum {
	ACC,
	DATA,
	ANS,
	CMD,
	HEAD
}msg_type_t;

typedef enum machine_role{
	MASTER,
	SUB_MASTER,
	DATA_MASTER,
	DATA_SERVER
}machine_role_e;

/*-------------------------POSITION START----------------------*/
typedef struct{
	int rank;
	uint64_t start;
	uint64_t end;
}position_des_t;
/*-------------------------POSITION E N D----------------------*/


/*-------------------------ROLE ALLOCATOR----------------------*/
typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version; //use to idetify specific message
	uint16_t unique_tag;
	int source;
	char ip[20];
	machine_role_e role_type;
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

/*------------------------COMMON MESSAGE-----------------------*/
typedef struct {
	int source;
	uint16_t operation_code;
	uint16_t transfer_version;
	int8_t rest[MAX_CMD_MSG_LEN - COMMON_MSG_HEAD];
}common_msg_t;

/*----------------------ACCEPT AESSAGE-------------------------*/
typedef struct {
	uint32_t op_status;
	uint32_t reserved;
}acc_msg_t;

/*-----------------------MESSAGE HEAD--------------------------*/
typedef struct {
	uint32_t len;
	uint32_t op_status;//status of last message
	uint64_t options;
}head_msg_t;

/*----------------------STOP SERVER MESSAGE---------------------*/
typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	int source;
	int tag;
	int reserved;
}stop_server_msg_t;

/*-------------------MASTER MESSAGE StRUCTURE------------------*/
#define FILE_NAME_MAX_LENGTH 255

/*
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
	char file_name[FILE_NAME_MAX_LENGTH];
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


//-------------------SUB_MASTER MESSAGE STRCTURE--------------------

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

//-------------------SUB_MASTER MESSAGE STRCTURE END-----------------

*/

/*-------------------DATA_MASTER MESSAGE STRUCTURE-------------------*/
/*
 * structure instruction of create file
 */
typedef struct client_create_file {
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t file_mode;
	uint16_t unique_tag;
	int source;
	char file_name[FILE_NAME_MAX_LENGTH + 1];
}client_create_file_t;

typedef struct file_ret {
	uint64_t offset; //This is offset from beginning
	uint64_t file_size;
	uint64_t dataserver_num;
}file_ret_t;

//typedef struct file_ret {
//	uint64_t offset; //This is offset from beginning
//	uint64_t file_size;
//	uint64_t dataserver_num;
//	//uint32_t chunks_num;
//
//	uint16_t *data_server_arr;
//	uint16_t *data_server_cnum;
//	//offset for each data server, this for writing and appending operation
//	uint64_t *data_server_offset;
//	uint64_t *data_server_len;
//
//	uint64_t *chunks_id_arr;
//}file_ret_t;

typedef struct file_sim_ret {
	uint16_t op_status;
}file_sim_ret_t;

typedef struct client_read_file {
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t unique_tag;
	uint16_t reserved;

	uint64_t read_size;
	uint64_t offset;
	int source;
	int tag;
	char file_name[FILE_NAME_MAX_LENGTH + 1];
}client_read_file_t;

typedef struct client_append_file {
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t unique_tag;
	uint16_t reserved;

	uint64_t write_size;
	int source;
	int tag;
	char file_name[FILE_NAME_MAX_LENGTH + 1];
}client_append_file_t;

typedef struct client_remove_file {
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t unique_tag;
	uint16_t reserved;

	char file_name[FILE_NAME_MAX_LENGTH + 1];
}client_remove_file_t;
/*
 * data server send heart beat to master
 */
typedef struct d_server_heart_beat {
	uint16_t operation_code;
	uint16_t transfer_version;
	int id;
}d_server_heart_beat_t;

//typedef struct c_d_block_data{
//	block_location block_info;
//	int8_t data[BLOCK_SIZE];
//}c_d_block_data_t;

typedef struct c_d_block_data{
	//block_location block_info;
	int8_t data[BLOCK_SIZE];
}c_d_block_data_t;

typedef struct c_d_register{
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t unique_tag;
	uint16_t reserved;

	uint64_t free_block;
	int source;
	char ip[20];
}c_d_register_t;


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
typedef struct {
	//64 bits
	uint16_t operation_code;
	uint16_t transfer_version;
	int unique_tag;//client must promise to offer a special tag to data server
	
	uint32_t offset;//offset from the begin of the first chunk that should be read
	uint32_t reserved;

	uint32_t read_len;//the length will read, just send to one data server
	uint32_t chunks_count;//the numbers of chunks will be read

	uint64_t chunks_id_arr[MAX_COUNT_CID_R];
}read_c_to_d_t;

//read and write use this message
typedef struct {
	uint32_t len;//data len in this package
	uint32_t offset;//form the beginning chunks

	uint32_t seqno;//number of this read
	int8_t tail;//if tail
	int8_t reserved[3];

	uint64_t data[MAX_COUNT_DATA];
}msg_data_t;

//origin write operation, do not support insert
typedef struct {
	//64 bits
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t unique_tag;
	uint32_t offset;
	//64 bits
	uint32_t write_len;
	uint32_t chunks_count;
	//64 bits aligned
	uint64_t chunks_id_arr[MAX_COUNT_CID_W];
}write_c_to_d_t;

//you can use this structure to insert text to a file
//if a message can not contain enough block_ids, you can send a serials messages
// and the seqno and tail is useful for this condition.
typedef struct {
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

/*----------------------ZOO KEEPER MESSAGE----------------------------*/
typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t znode_type;
	uint16_t unique_tag;

	char path[ZOO_PATH_MAX_LEN];
	char data[ZOO_MAX_DATA_LEN];
}zoo_create_znode_t;

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t version;
	uint16_t unique_tag;

	char path[ZOO_PATH_MAX_LEN];
}zoo_delete_znode_t;

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t version;
	uint16_t unique_tag;

	char path[ZOO_PATH_MAX_LEN];
	char data[ZOO_MAX_DATA_LEN];
}zoo_set_znode_t;

//watch return message will send with an unique code
typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t watch_flag;
	uint16_t watch_code;

	uint16_t unique_tag;

	char path[ZOO_PATH_MAX_LEN];
}zoo_exists_znode_t;

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;
	uint16_t watch_flag;
	uint16_t watch_code;

	uint16_t unique_tag;

	char path[ZOO_PATH_MAX_LEN];
}zoo_get_znode_t;

typedef struct {
	uint16_t operation_code;
	uint16_t transfer_version;

	uint16_t unique_tag;

	char path[ZOO_PATH_MAX_LEN];
}zoo_get_children_t;
/*--------------------ZOO KEEPER MESSAGE END--------------------------*/

/*-------------------COMMON MESSAGE FUNCTIONS-------------------------*/
void common_msg_dup(void *dest, void *source);
void recv_common_msg(common_msg_t* msg, int source, int tag);
int get_source(common_msg_t* msg);
uint16_t get_transfer_version(common_msg_t* msg);
uint16_t get_operation_code(common_msg_t* msg);

/*------------------------MESSAGE FUNCTIONS---------------------------*/
//You can choose MPI or sockets from send message
int send_cmd_msg(void* msg, int dst, uint32_t len);
//this function should be modified later
void send_data_msg(void* msg, int dst, int tag, uint32_t len);
void send_msg(void* msg, int dst, int tag, int len);
void send_acc_msg(void* msg, int dst, int tag, int status);
void send_head_msg(void* msg, int dst, int tag);

void recv_acc_msg(void* msg, int source, int tag);
//this function should be modified later
void recv_data_msg(void* msg, int source, int tag, uint32_t len);
void recv_head_msg(void* msg, int source, int tag);
void recv_msg(void* msg, int source, int tag, int len);

#endif /* SRC_COMMON_COMMUNICATION_MESSAGE_H_ */
