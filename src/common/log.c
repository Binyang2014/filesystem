/**
 * create on: 2015.3.27
 * author: Andrew M.Rudoff
 * modified: Binyang
 *
 * implement of log.h
 */

#include "../global.h"
#include "log.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>		/* ANSI C header file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>


static const char* level_name[] = {"LOG_TRACE", "LOG_DEBUG", "LOG_INFO", "LOG_WARN", "LOG_ERROR", "LOG_OFF"};
static logger_t global_logger;

//need to set log_to_stdout bit
int log_init(const char* dir, log_level_t level)
{
	int dir_len, temp;

	if(pthread_mutex_init(&global_logger.log_write_mutex, NULL) != 0)
	{
		fprintf(stderr, "init log write mutex\n");
		exit(1);
	}
	global_logger.level = level;

	//set global_logger.log_to_stdout to right value
	global_logger.log_to_stdout = 1;
	dir_len = strlen(dir);
	if(dir_len > LOG_MAX_PATH)
	{
		fprintf(stderr, "directoty path is too long\n");
		exit(1);
	}
	strcpy(global_logger.full_log_path, dir);
	if(dir[0] == '\0')
		global_logger.log_to_stdout = 1;
	else if((temp = access(dir, F_OK)) != 0)
	{
		if(mkdir(dir, S_IRWXU) != 0)
		{
			fprintf(stderr, "create fold faild, will put log to stdout\n");
			global_logger.log_to_stdout = 1;
		}
		else global_logger.log_to_stdout = 0;
	}
	else
		global_logger.log_to_stdout = 0;

	//set log file to empty file
	strcat(global_logger.full_log_path, "/MemFs.log");

	global_logger.fp = global_logger.log_to_stdout ? 
		stdout : fopen(global_logger.full_log_path, "w");
	if(!global_logger.log_to_stdout)
		fclose(global_logger.fp);

	global_logger.fp = global_logger.log_to_stdout ? 
		stdout : fopen(global_logger.full_log_path, "a");

	return 0;
}

//low level logging, call by log_write
static void __write_log(log_level_t level, char* msg)
{
	FILE*fp = global_logger.fp;
	char buf[64];

	if(level < global_logger.level)
		return;

	if(!fp)
	{
		fprintf(stderr, "file is not opened");
		return;
	}
	else
	{
		int off;
		struct timeval tv;
		pid_t pid = getpid();
		pthread_t tid = pthread_self();

		gettimeofday(&tv, NULL);
		off = strftime(buf, sizeof(buf), "%d %b %H:%M:%S.", localtime(&tv.tv_sec));
		snprintf(buf+off, sizeof(buf)-off, "%03d", (int)tv.tv_usec/1000);

		if(LOG_THREAD_SAFE)
		{
			pthread_mutex_lock(&global_logger.log_write_mutex);
			fprintf(fp, "%d %u %s %s %s\n", (int)pid, (unsigned int)tid, buf, level_name[level], msg);
			pthread_mutex_unlock(&global_logger.log_write_mutex);
		}
		else
			fprintf(fp, "%d %s %s %s\n", (int)pid, buf, level_name[level], msg);
		fflush(fp);
	}
	return;
}

//This function write log to file system
void log_write(log_level_t level, const char* fmt, ...)
{
	va_list ap;
	char msg[MAX_LOG_MSG_LEN + 1];

	if(level < global_logger.level)
		return;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	__write_log(level, msg);
	return;
}

//This function is aimed to clear resouce that has used
void log_close()
{
	if(!global_logger.log_to_stdout)
		fclose(global_logger.fp);
	return;
}

void err_ret(const char *fmt, ...)
{
	va_list ap;
	char msg[MAX_LOG_MSG_LEN + 1];
	int errno_save ,n;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	errno_save = errno;
	n = strlen(msg);
	snprintf(msg + n, MAX_LOG_MSG_LEN - n, ": %s", strerror(errno_save));
	__write_log(LOG_INFO, msg);
	return;
}

/* Fatal error related to system call
 * Print message and terminate */

void err_sys(const char *fmt, ...)
{
	va_list ap;
	char msg[MAX_LOG_MSG_LEN + 1];
	int errno_save ,n;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	errno_save = errno;
	n = strlen(msg);
	snprintf(msg + n, MAX_LOG_MSG_LEN - n, ": %s", strerror(errno_save));
	__write_log(LOG_ERR, msg);
	exit(1);
}

/* Fatal error related to system call
 * Print message, dump core, and terminate */

void err_dump(const char *fmt, ...)
{
	va_list ap;
	char msg[MAX_LOG_MSG_LEN + 1];
	int errno_save ,n;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	errno_save = errno;
	n = strlen(msg);
	snprintf(msg + n, MAX_LOG_MSG_LEN - n, ": %s", strerror(errno_save));
	__write_log(LOG_ERR, msg);
	abort();        /* dump core and terminate */
	exit(1);        /* shouldn't get here */
}

/* Nonfatal error unrelated to system call
 * Print message and return */

void err_msg(const char *fmt, ...)
{
	va_list	    ap;
	char msg[MAX_LOG_MSG_LEN + 1];

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	__write_log(LOG_INFO, msg);
	return;
}

/* Fatal error unrelated to system call
 * Print message and terminate */

void err_quit(const char *fmt, ...)
{
	va_list ap;
	char msg[MAX_LOG_MSG_LEN + 1];

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	__write_log(LOG_INFO, msg);
	exit(1);
}
