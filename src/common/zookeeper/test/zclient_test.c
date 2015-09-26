#include "zookeeper.h"
#include "zmalloc.h"

int main()
{
	zclient_t *zclient;

	zclient = create_zclient(sizeof(zclient_t));
	destroy_zclient(zclient);
	return 0;
}
