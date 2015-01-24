/*
 * datasever.h
 *
 * Created on 2015.1.23
 * Author:binyang
 */

#ifndef _FILESYSTEM_CLIENT_DATASERVER_H_
#define _FILESYSTEM_CLIENT_DATASERVER_H_
#define MAX_ALLOC_SIZE 10000
//数据节点的结构体,是不是应该有个global的文件
struct chunk
{
};

struct chunk_list
{
};
//根据文件系统的结构建立空闲链表，已使用的链表等等
//注册本台机器
void init_data_sterver();
void init_lists();
void check_limits();//查看是否还允许分配
void get_current_imformation();
void alloc_a_chunk();
void delete_a_chunk();
void write_chunks();
void read_chunks();
void adjust_lists();
#endif
