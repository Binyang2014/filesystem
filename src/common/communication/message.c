/**
 * create on: 2015.3.29
 * author: Binyang
 *
 * This file offer complete some basic message functions
 */
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"
#include "mpi_communication.h"

void common_msg_dup(void *dest, void *source){
	memcpy(dest, source, sizeof(common_msg_t));
}

//if source == -1 receive any source if tag == -1 receive any tag
void recv_common_msg(common_msg_t* msg, int source, int tag)
{
	mpi_status_t status;
	mpi_recv(MSG_COMM_TO_CMD(msg), source, tag, MAX_CMD_MSG_LEN, &status);
	msg->source = status.source;
}

int get_source(common_msg_t* msg)
{
	return msg->source;
}

uint16_t get_transfer_version(common_msg_t* msg)
{
	return msg->transfer_version;
}

uint16_t get_operation_code(common_msg_t* msg)
{
	return msg->operation_code;
}

void send_cmd_msg(void* msg, int dst, int tag)
{
	//ifdefine mpi_communicate
	mpi_send(msg, dst, tag, MAX_CMD_MSG_LEN);
	//else use other tools such as sockets
}

void send_data_msg(void* msg, int dst, int tag, uint32_t len)
{
	//maybe need compress here
	mpi_send(msg, dst, tag, MAX_DATA_MSG_LEN);
}

void send_acc_msg(void* msg, int dst, int tag, int status)
{
	acc_msg_t* acc_msg = (acc_msg_t* )acc_msg;
	if(status != ACC_IGNORE)
		acc_msg->op_status = status;
	mpi_send(msg, dst, tag, sizeof(acc_msg_t));
	//sockets functions
}

void send_head_msg(void* msg, int dst, int tag)
{
	//mpi function only
	mpi_send(msg, dst, tag, sizeof(head_msg_t));
}

void send_msg(void* msg, int dst, int tag, int len)
{
	mpi_send(msg, dst, tag, len);
	//else can use sockets
}

void recv_data_msg(void* msg, int source, int tag, uint32_t len)
{
	//only mpi function now
	mpi_status_t status;
	mpi_recv(msg, source, tag, MAX_CMD_MSG_LEN, &status);
}

void recv_acc_msg(void* msg, int source, int tag)
{
	mpi_status_t status;
	mpi_recv(msg, source, tag, sizeof(acc_msg_t), &status);
}

void recv_head_msg(void* msg, int source, int tag)
{
	mpi_status_t status;
	mpi_recv(msg, source, tag, sizeof(head_msg_t), &status);
}

void recv_msg(void* msg, int source, int tag, int len)
{
	//only mpi function here
	mpi_status_t status;
	mpi_recv(msg, source, tag, len, &status);
}
