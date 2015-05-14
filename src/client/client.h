/*
 * clinet.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */


#ifndef SRC_MAIN_CLIENT_H_
#define SRC_MAIN_CLIENT_H_

/*===============api declaration================*/
int  client_init();
int  client_destroy();

//提供给本地程序的服务接口
/***********client service****************************/
int local_create_file(char *local_name, char *system_name);

#endif /* SRC_MAIN_CLIENT_H_ */
