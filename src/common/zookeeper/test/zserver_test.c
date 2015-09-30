#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "zookeeper.h"
#include "log.h"
#include "message.h"
#include "zmalloc.h"

int main()
{
	zserver_t *zserver;
	zoo_create_znode_t create_msg;
	zoo_set_znode_t set_msg;
	zoo_get_znode_t get_msg;
	zoo_exists_znode_t exists_msg;
	zoo_delete_znode_t delete_msg;
	common_msg_t *common_msg;
	void* cmd_msg;

	log_init("", LOG_DEBUG);
	zserver = create_zserver(1);
	zserver->op->zserver_start(zserver);
	common_msg = zmalloc(sizeof(common_msg_t));

	//create znode parent
	create_msg.operation_code = ZOO_CREATE_CODE;
	create_msg.znode_type = 1;
	create_msg.unique_tag = 13;
	strcpy((char *)create_msg.path, "/data");
	strcpy((char *)create_msg.data, "data");
	common_msg->source = 1;
	cmd_msg = MSG_COMM_TO_CMD(common_msg);
	memcpy(cmd_msg, &create_msg, sizeof(zoo_create_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	//create znode
	create_msg.operation_code = ZOO_CREATE_PARENT_CODE;
	create_msg.znode_type = 1;
	create_msg.unique_tag = 13;
	strcpy((char *)create_msg.path, "/source/m.bat");
	strcpy((char *)create_msg.data, "source bat");
	common_msg->source = 1;
	cmd_msg = MSG_COMM_TO_CMD(common_msg);
	memcpy(cmd_msg, &create_msg, sizeof(zoo_create_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	//get znode data
	get_msg.operation_code = ZOO_GET_CODE;
	get_msg.watch_flag = 0;
	get_msg.watch_code = 0;
	get_msg.unique_tag = 13;
	strcpy((char *)get_msg.path, "/source/m.bat");
	common_msg->source = 1;
	cmd_msg = MSG_COMM_TO_CMD(common_msg);
	memcpy(cmd_msg, &get_msg, sizeof(zoo_get_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	//set znode data
	set_msg.operation_code = ZOO_SET_CODE;
	set_msg.version = -1;
	set_msg.unique_tag = 13;
	strcpy((char *)set_msg.path, "/source/m.bat");
	strcpy((char *)set_msg.data, "source batx");
	common_msg->source = 1;
	cmd_msg = MSG_COMM_TO_CMD(common_msg);
	memcpy(cmd_msg, &set_msg, sizeof(zoo_set_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	//get znode data with watch flag
	get_msg.operation_code = ZOO_GET_CODE;
	get_msg.watch_flag = 1;
	get_msg.watch_code = 23;
	get_msg.unique_tag = 13;
	strcpy((char *)get_msg.path, "/source/m.bat");
	common_msg->source = 1;
	cmd_msg = MSG_COMM_TO_CMD(common_msg);
	memcpy(cmd_msg, &get_msg, sizeof(zoo_get_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	//exist znode
	exists_msg.operation_code = ZOO_EXISTS_CODE;
	exists_msg.watch_flag = 1;
	exists_msg.watch_code = 27;
	exists_msg.unique_tag = 13;
	strcpy((char *)exists_msg.path, "/source/m.bat");
	common_msg->source = 1;
	cmd_msg = MSG_COMM_TO_CMD(common_msg);
	memcpy(cmd_msg, &exists_msg, sizeof(zoo_exists_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	//set znode again
	set_msg.operation_code = ZOO_SET_CODE;
	set_msg.version = -1;
	set_msg.unique_tag = 13;
	strcpy((char *)set_msg.path, "/source/m.bat");
	strcpy((char *)set_msg.data, "source batxx");
	common_msg->source = 1;
	cmd_msg = MSG_COMM_TO_CMD(common_msg);
	memcpy(cmd_msg, &set_msg, sizeof(zoo_set_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	//set znode again no more message
	set_msg.operation_code = ZOO_SET_CODE;
	set_msg.version = -1;
	set_msg.unique_tag = 13;
	strcpy((char *)set_msg.path, "/source/m.bat");
	strcpy((char *)set_msg.data, "source baxxx");
	common_msg->source = 1;
	cmd_msg = MSG_COMM_TO_CMD(common_msg);
	memcpy(cmd_msg, &set_msg, sizeof(zoo_set_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	//exist znode
	exists_msg.operation_code = ZOO_EXISTS_CODE;
	exists_msg.watch_flag = 2;
	exists_msg.watch_code = 23;
	exists_msg.unique_tag = 13;
	strcpy((char *)exists_msg.path, "/source/m.bat");
	common_msg->source = 1;
	cmd_msg = MSG_COMM_TO_CMD(common_msg);
	memcpy(cmd_msg, &exists_msg, sizeof(zoo_exists_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	//delete znode
	delete_msg.operation_code = ZOO_DELETE_CODE;
	delete_msg.version = -1;
	delete_msg.unique_tag = 13;
	strcpy((char *)delete_msg.path, "/source/m.bat");
	common_msg->source = 1;
	memcpy(cmd_msg, &delete_msg, sizeof(zoo_delete_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	//exist znode
	exists_msg.operation_code = ZOO_EXISTS_CODE;
	exists_msg.watch_flag = 2;
	exists_msg.watch_code = 23;
	exists_msg.unique_tag = 13;
	strcpy((char *)exists_msg.path, "/source/m.bat");
	common_msg->source = 1;
	cmd_msg = MSG_COMM_TO_CMD(common_msg);
	memcpy(cmd_msg, &exists_msg, sizeof(zoo_exists_znode_t));
	zserver->op->zput_request(zserver, common_msg);
	usleep(500);

	zserver->op->zserver_stop(zserver);
	destroy_zserver(zserver);
	zfree(common_msg);
	return 0;
}
