#include <stdlib.h>
#include <stdio.h>
#include "client.h"
#include "log.h"

char buf[] = "1 2 3 4 5 6 7 8 9 10";
char read_buf[30] = {0};

int main()
{
	int fd;

	log_init("", LOG_DEBUG);
	if(init_client() < 0)
	{
		exit(1);
	}
	fd = f_open("/temp/a.txt", CREATE_TEMP | RDWR, RUSR | WUSR);
	if(fd < 0)
	{
		log_write(LOG_ERR, "open file failed");
		exit(1);
	}
	f_close(fd);
	log_write(LOG_DEBUG, "file open successfully");
	f_open("temp/a.txt", RDWR);
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
	return 0;
}
