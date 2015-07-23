#include "../log.h"
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

void* write_log(void* arg)
{
	log_write(LOG_INFO, "This is in thread message");
	log_write(LOG_DEBUG, "log file thread function works well");
	errno = 15;
	err_ret("err_ret");
	err_msg("err_msg");

	return NULL;
}

int main()
{
	char* s = "/home/binyang/Program/filesystem/src/common/test/1";
	pthread_t tid;

	printf("The dir is %s", s);
	log_init("", LOG_DEBUG);

	pthread_create(&tid, NULL, write_log, NULL);

	log_write(LOG_INFO, "This is a test message");
	log_write(LOG_DEBUG, "log file works well %s", s);
	errno = 10;
	err_ret("err_ret");
	err_msg("err_msg");
	pthread_join(tid, NULL);

	return 0;
}
