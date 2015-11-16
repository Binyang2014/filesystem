#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include "data_master.h"
#include "machine_role.h"
#include "data_master_request.h"
#include "zmalloc.h"
#include "log.h"
#include "message.h"
#include "mpi_communication.h"

static c_d_register_t* get_register_cmd(int rank)
{
	c_d_register_t *re = zmalloc(sizeof(*re));
	re->free_block = 1024;
	re->source = rank;
	strcpy(re->ip, "10.4.13.222");
	re->operation_code = REGISTER_TO_DATA_MASTER;
	re->unique_tag = CMD_TAG;
	return re;
}

static client_create_file_t *get_create_file_cmd(int rank)
{
	client_create_file_t *cmd = zmalloc(sizeof(*cmd));
	cmd->file_mode = TEMPORARY_FILE;
	strcpy(cmd->file_name, "/home/ron");
	cmd->source = rank;
	cmd->unique_tag = CMD_TAG;
	cmd->operation_code = CREATE_TEMP_FILE_CODE;
	return cmd;
}

static client_append_file_t *get_append_file_cmd(int rank)
{
	client_append_file_t *cmd = zmalloc(sizeof(*cmd));
	strcpy(cmd->file_name, "/home/ron");
	cmd->source = rank;
	cmd->operation_code = APPEND_TEMP_FILE_CODE;
	cmd->unique_tag = CMD_TAG;
	cmd->write_size = 1 << 20;
	return cmd;
}

static client_read_file_t *get_read_file_cmd(int rank)
{
	client_read_file_t *cmd = zmalloc(sizeof(*cmd));
	strcpy(cmd->file_name, "/home/ron");
	cmd->source = rank;
	cmd->offset = 0;
	cmd->read_size = 1 << 10;
	cmd->operation_code = READ_TEMP_FILE_CODE;
	cmd->unique_tag = CMD_TAG;
	return cmd;
}

int main(argc, argv)
	int argc;char ** argv; {
	int rank;

	mpi_init_with_thread(&argc, &argv);
	rank = get_mpi_rank();

	if(rank == 0){
		map_role_value_t *role = zmalloc(sizeof(*role));
		role->group_size = 2;
		strcpy(role->ip, "10.4.13.22");
		role->rank = 0;
		role->master_rank = 0;
		role->type = DATA_MASTER;
		strcpy(role->master_ip, "10.4.13.22");
		data_master_t *master = create_data_master(role);
		data_master_init(master);
	}else{
		c_d_register_t *re = get_register_cmd(rank);
		data_master_request_t *request = create_data_master_request(rank, 0, CMD_TAG);
		request->op->register_to_master(request, re);

		client_create_file_t *c_c_f = get_create_file_cmd(rank);
		request->op->create_tmp_file(request, c_c_f);

		client_append_file_t *c_a_f = get_append_file_cmd(rank);
		request->op->append_temp_file(request, c_a_f);

		client_read_file_t *c_r_f = get_read_file_cmd(rank);
		request->op->read_temp_file(request, c_r_f);

	}
	mpi_finish();
	return 0;
}

