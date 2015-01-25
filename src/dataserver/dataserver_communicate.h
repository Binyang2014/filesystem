/**
 * dataserver_communicate.h
 *
 * created on 2015.1.25
 * author binyang
 *
 * this file define some function used to communicate with master and client
 */
#ifndef _DATASERVER_COMMNICATE_H_
#define _DARASERVER_COMMNICATE_H_
struct message
{
	int src;
	int des;
};
//之后还有一系列的消息格式定义。。。这部分消息格式定义是不是应该放在外面而不是dataserver中
void send();
void recive();
//...other message passing functions
#endif
