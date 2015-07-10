/**
 * log.h
 * create on: 2015.3.27
 * modified on: 2015.7.9
 * author: Andrew M.Rudoff
 * modified: Binyang
 *
 * This file is decided to log messages come from C program
 * one program has one log file, and log function must be thread-safe. 
 * Programmer must make true that one process only has one logger
 * There are some errors handling functions, just use functions in this file to deal with potential
 * error. I copy these code from Unix Network Programming
 */

#ifndef _LOG_H_
#define _LOG_H_

#define LOG_THREAD_SAFE 1
#define MAX_LOG_MSG_LEN 1024
#define LOG_MAX_PATH 512

#include <stdio.h>
#include <pthread.h>

typedef enum log_level
{
	LOG_TRACE = 0,
	LOG_DEBUG = 1,
	LOG_INFO = 2,
	LOG_WARN = 3,
	LOG_ERR = 4,
	LOG_OFF = 5,
}log_level_t;

struct logger
{
	pthread_mutex_t log_write_mutex;
	FILE* fp;
	log_level_t level;
	char full_log_path[LOG_MAX_PATH + 50];//path is a abs path
	char log_to_stdout;
};

typedef struct logger logger_t;

int log_init(const char* dir, log_level_t level);//at the very beginnning
void log_write(log_level_t level, const char* fmt, ...);//multiple threads may write to this log file
void log_close();

void err_ret(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_dump(const char *fmt, ...);
void err_msg(const char *fmt, ...);
void err_quit(const char *fmt, ...);
#endif
