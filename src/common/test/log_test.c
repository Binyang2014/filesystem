#include "../log.h"
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

int main()
{
	char* s = "dir/home/binyang/Program/filesystem/src/common/test";
	printf("The dir is %s", s);
	log_init(s, LOG_DEBUG);
	log_write(LOG_INFO, "This is a test message");
	log_write(LOG_DEBUG, "log file works well %s", s);
	errno = 10;
	err_ret("err_ret");
	err_msg("err_msg");
	err_quit("err_quit");

	return 0;
}
