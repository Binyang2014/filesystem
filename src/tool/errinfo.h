/**
 * error.h
 * create on: 2015.3.27
 * author: Andrew M.Rudoff
 *
 * This is errors handling functions, just use functions in this file to deal with potential
 * error. I copy these code from Unix Network Programming
 */

#ifndef _ERRORINFO_H_
#define _ERRORINFO_H_

void err_ret(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_dump(const char *fmt, ...);
void err_msg(const char *fmt, ...);
void err_quit(const char *fmt, ...);
#endif
