/*
  * created on: 2015.12.10
  * author: binyang

  * This file is going provide IPC by using message queue
*/
#ifndef MSG_IPC_H_
#define MSG_IPC_H_
#define COMMON_TYPE 13

#include <stdint.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "global.h"

#define MSG_TEXT_PTR(msg) ((msg) + sizeof(long))
#define MSG_TYPE(msg) (*((long *)msg))

void *create_message(long type, size_t nbytes);
int create_msq(uint32_t key, mode_t mode);
int open_msq(uint32_t key, mode_t mode);
void remove_msq(int msqid);
ssize_t msq_read(int msqid, void *message, size_t nbytes);
ssize_t msq_write(int msqid, void *message, size_t nbytes);
ssize_t m_read(int msqid, void *buff, size_t nbytes);
ssize_t m_write(int msqid, void *buff, size_t nbytes);
#endif
