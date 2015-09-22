#include <stdio.h>
#include "zookeeper.h"
#include "log.h"

int main()
{
	zserver_t *zserver;
	zoo_create_znode_t create_msg;

	log_init("", LOG_DEBUG);
	zserver = create_zserver(1);
	zserver->op->zserver_start(zserver);

	create_msg.operation_code = ZOO_CREATE_CODE;
	create_msg.znode_type = 1;

	zserver->op->zserver_stop(zserver);
	destroy_zserver(zserver);
	return 0;
}
