#include "http.h"
#ifndef EPOLL
#define EPOLL

static void usage(const char *proc)
{
	printf("Usage:%s[local_ip][local_port]\n",proc);
}

int main(int argc,char *argv[])
{
	if(argc != 3)
	{
		usage(argv[0]);
		return 1;
	}
	int listen_sock = startup(argv[1],atoi(argv[2]));
	if(listen_sock < 0)
	{
		return 2;
	}
	//daemon(0,0);
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
#ifdef MULTITHREAD
	while(1)
	{
		int new_sock = accept(listen_sock,\
				(struct sockaddr*)&client,\
					   &len);
		if(new_sock < 0)
		{
			perror("accept");
			continue;
		}
		printf("get a client [%s:%d]\n",\
				inet_ntoa(client.sin_addr),\
				ntohs(client.sin_port));
		pthread_t id;
		pthread_create(&id,NULL,handler_request,(void*)new_sock);
		pthread_detach(id);
	}
#endif //MULTITHREAD
#ifdef EPOLL
	int efd;
	struct epoll_event event;
	struct epoll_event* events;
	int max_epoll_size = 65535;
	//get_config("max_epoll_size",max_epoll_size);
	//efd=epoll_create(atoi(max_epoll_size));
	efd = epoll_create(max_epoll_size);
	if(efd<0){
		perror("epoll_create!");
		exit(1);
	}
	event.data.fd = listen_sock;
	event.events = EPOLLIN | EPOLLET;
	int ret = epoll_ctl(efd,EPOLL_CTL_ADD,listen_sock,&event);
	if(ret < 0){
		perror("epoll_ctl error\n");
		exit(1);
	}
	//setnonblocking(efd);
	events=(struct epoll_event*)calloc(max_epoll_size,sizeof(event));
	while(1){
		int n,i,new_sock;
again:
		n = epoll_wait(efd,events,max_epoll_size,-1);
		if(n < 0){
			if(errno == EINTR)
				goto again;
			perror("epoll_wait");
			exit(1);
		}
		for(i=0;i<n;++i){
			if((events[i].events & EPOLLERR) ||\
			   (events[i].events & EPOLLHUP) ){//服务器出错
				perror("epoll_wait");
				close(events[i].data.fd);
				continue;
			}
			else if(listen_sock == events[i].data.fd){
				 new_sock=accept(listen_sock,(struct sockaddr*)&client,&len);
				if(new_sock < 0){
					perror("accept error");
					exit(1);
				}
				//setnonblocking(new_sock);
				event.data.fd = new_sock;
				event.events = EPOLLIN | EPOLLET;
				ret = epoll_ctl(efd,EPOLL_CTL_ADD,new_sock,&event);
				if(ret < 0){
					perror("epoll_ctl error");
					exit(1);
				}
			}
			else if(events[i].events&EPOLLIN){//
				new_sock = events[i].data.fd;
				ret = epoll_ctl(efd,EPOLL_CTL_DEL,new_sock,&event);
				if(ret < 0){
					perror("epoll_ctl error");
					exit(1);
				}
				handler_request(events[i].data.fd);
				close(events[i].data.fd);
				event.data.fd = new_sock;
				event.events = EPOLLIN | EPOLLET;
				close(new_sock);
			}
			else if(events[i].events&EPOLLOUT){//写事件就绪
			}else{
			}
		}
	}
	free(events);
#endif
	return 0;
}
#endif
