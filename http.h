#ifndef _HTTP_H_
#define _HTTP_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/sendfile.h>
#include <sys/epoll.h>
#include <errno.h>

#define SIZE 1024
#define NOTICE 0
#define WARNING 1
#define FATAL 2

void print_log(const char* msg,int level);
int startup(const char* ip,int port);
void* handler_request(void * arg);
void echo_error(int fd,int erroe_num);
int echo_www(int fd,const char* path,int size);
int exe_cgi(int fd,const char *method,\
		const char* path,const char*query_string);


#endif
