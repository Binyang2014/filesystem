#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "log.h"
void user_func()
{
	int fd;
	char buf[] = "1 2 3 4 5 6 7 8 9 10";
	char read_buf[30] = {0};

	printf("-------------hello--------------\n");
	fd = f_open("/temp/a.txt", CREATE_TEMP | RDWR, RUSR | WUSR);
	if(fd < 0)
	{
		log_write(LOG_ERR, "open file failed");
		exit(1);
	}
	log_write(LOG_DEBUG, "file open successfully");
	f_close(fd);
	f_open("/temp/a.txt", RDWR);
	f_append(fd, buf, sizeof(buf));
	memset(buf, 0, 21);
	f_read(fd, read_buf, 21);
	printf("1data is %s\n", read_buf);
	memset(buf, 0, 21);

	f_read(fd, read_buf, 21);
	printf("2data is %s\n", read_buf);
	memset(buf, 0, 21);
	f_read(fd, read_buf, 21);
	printf("3data is %s\n", read_buf);
	memset(buf, 0, 21);

	f_read(fd, read_buf, 21);
	printf("4data is %s\n", read_buf);




	f_close(fd);

	//read again
	f_open("/temp/a.txt", RDWR);
	f_read(fd, read_buf, 21);
	printf("data is %s\n", read_buf);
	f_close(fd);
}
