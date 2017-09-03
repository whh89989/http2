#include "http.h"
#ifndef EPOLL_LT
#define EPOLL_LT

typedef struct ep_buff{
	int fd;
	char buff[1024];
}ep_buff_t,*ep_buff_p;
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
	
#ifdef MULTITHREAD
	printf("MULTITHREAD\n");
	while(1)
	{
    	struct sockaddr_in client;
    	socklen_t len = sizeof(client);
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
#ifdef EPOLL_LT
	printf("EPOLL_LT\n");
	int max_epoll_size = 65535;
	int efd = epoll_create(max_epoll_size);

	struct epoll_event ev;
	ev.data.fd = listen_sock;
	ev.events = EPOLLIN;
	
	//struct epoll_event* events;
	if(efd<0){
		perror("epoll_create!");
		exit(1);
	}
    //	ev.data.ptr = alloc_ep_buff(listen_sock);

	int ret = epoll_ctl(efd,EPOLL_CTL_ADD,listen_sock,&ev);
	if(ret < 0){
		perror("epoll_ctl error\n");
		exit(1);
	}
	struct epoll_event events[max_epoll_size];
	setnonblocking(efd);
	//events=(struct epoll_event*)calloc(max_epoll_size,sizeof(ev));
	while(1){
		int n,i;
again:
		n = epoll_wait(efd,events,max_epoll_size,-1);
		if(n < 0){
			//if(errno == /*EAGAIN*/EINTR)
			//	goto again;
			perror("epoll_wait");
			//exit(1);
			break;
		}
		else if(n == 0)	{
			perror("timeout...\n");
			break;
		}
		for(i=0;i<n;++i){
			//int sock = ((ep_buff_p)(events[i].data.ptr));
			int sock = events[i].data.fd;

			/*if((events[i].events & EPOLLERR) ||\
			   (events[i].events & EPOLLHUP) ){//服务器出错
				perror("epoll_wait");
				close(events[i].data.fd);
				continue;
			}else*/ if(listen_sock == sock && \
					(events[i].events & EPOLLIN)){
            	 struct sockaddr_in client;
            	 socklen_t len = sizeof(client);
				 int new_sock=accept(listen_sock,\
						 (struct sockaddr*)&client,&len);
				if(new_sock < 0){
					//if(errno != EAGAIN && errno != ECONNABORTED\
							&& errno != EPROTO && errno != EINTR)
					perror("accept error");
					continue;
				//	exit(1);
				}
				printf("get client :[%s:%d]\n",\
						inet_ntoa(client.sin_addr),
						ntohs(client.sin_port));
				setnonblocking(new_sock);
				ev.data.fd = new_sock;
				ev.events = EPOLLIN;
				//ev.data.ptr = alloc_ep_buff(new_sock);
				ret = epoll_ctl(efd,EPOLL_CTL_ADD,new_sock,&ev);
				if(ret < 0){
					perror("epoll_ctl error");
					exit(1);
				}
				printf("client over\n");
			}
			else if(sock != listen_sock){
				if(events[i].events & EPOLLIN){//
				//int new_sock = events[i].data.fd;
			    //ret = epoll_ctl(efd,EPOLL_CTL_DEL,sock,&ev);
				//if(ret < 0){
				//	perror("epoll_ctl error");
				//	exit(1);
				//}
				printf("hander...\n");
				handler_request(events[i].data.fd);//改过

				close(events[i].data.fd);
				ev.data.fd = sock;
			    ev.events = events[i].events | EPOLLIN ;
			    ret = epoll_ctl(efd,EPOLL_CTL_DEL,sock,&ev);
				close(sock);
				printf("close over...\n");
			}
			else if(events[i].events&EPOLLOUT){//写事件就绪
				printf("write...\n");
			}else{
				printf("what...\n");
			}
		}
		}
	}
	free(events);
#endif //EPOLL LT
#ifdef EPOLL_ET
	int efd;
	struct epoll_event ev;
	struct epoll_event* events;
	int max_epoll_size = 65535;
	//get_config("max_epoll_size",max_epoll_size);
	//efd=epoll_create(atoi(max_epoll_size));
	efd = epoll_create(max_epoll_size);
	if(efd<0){
		perror("epoll_create!");
		exit(1);
	}
	ev.data.fd = listen_sock;
	ev.events = EPOLLIN;
	int ret = epoll_ctl(efd,EPOLL_CTL_ADD,listen_sock,&ev);
	if(ret < 0){
		perror("epoll_ctl error\n");
		exit(1);
	}
	setnonblocking(efd);
	events=(struct epoll_event*)calloc(max_epoll_size,sizeof(ev));
	while(1){
    	struct sockaddr_in client;
    	socklen_t len = sizeof(client);
		int n,i,new_sock;
again:
		n = epoll_wait(efd,events,max_epoll_size,-1);
		if(n < 0){
			if(errno == /*EAGAIN*/EINTR)
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
				setnonblocking(new_sock);
				ev.data.fd = new_sock;
				ev.events = EPOLLIN | EPOLLET;
				ret = epoll_ctl(efd,EPOLL_CTL_ADD,new_sock,&ev);
				if(ret < 0){
					perror("epoll_ctl error");
					exit(1);
				}
			}
			else if(events[i].events & EPOLLIN){//
				new_sock = events[i].data.fd;
				ret = epoll_ctl(efd,EPOLL_CTL_DEL,new_sock,&ev);
				if(ret < 0){
					perror("epoll_ctl error");
					exit(1);
				}
				handler_request(events[i].data.fd);
				close(events[i].data.fd);
				ev.data.fd = new_sock;
				ev.events = events[i].events | EPOLLIN | EPOLLET;
				close(new_sock);
			}
			else if(events[i].events&EPOLLOUT){//写事件就绪
			}else{
			}
		}
	}
#endif
	return 0;
	}
#endif
