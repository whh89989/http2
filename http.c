#include "http.h"

int startup(const char*ip,int port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock<0){
		perror("socket");
		return -2;
	}

	int opt = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);
	if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0){
		perror("bind");
		return -3;
	}
	if(listen(sock,10) < 0){
		perror("listen");
		return -4;
	}

	return sock;
}

static int get_line(int fd, char *buf,int len)  //按行提取
{
	char c = '\0';
	int i = 0;
	while(c != '\n'&&i < len-1)
	{
		ssize_t s = recv(fd,&c,1,0);
		if(s > 0)
		{
			if(c == '\r')
			{
				recv(fd,&c,1,MSG_PEEK);//窥探，不拿走
				if(c == '\n')
				{
					recv(fd,&c,1,0);//直接拿走
				}
				else
				{
					c = '\n';//换成\n
				}
			}
			buf[i++] = c;
		}
	}
	buf[i] = 0;
	return i;
}

void print_log(const char *msg,int level)
{
	const char *level_msg[] = {
		"NOTICE",
		"WARNING",
		"FATAL",
	};
	printf("[%s][%s]\n",msg,level_msg[level]);
}
static void show_404(fd)
{
	const char *echo_header = "HTTP/1.0 404 Not Found!";
	send(fd,echo_header,strlen(echo_header),0);
	const char *type = "Content-Type:text/html;charset=ISO-8859-1\r\n";
	send(fd,type,strlen(type),0);
	const char *blank_line = "\r\n";
	send(fd,blank_line,strlen(blank_line),0);

	const char *msg = "<html><h1>Page Not Found!</h1></html>\r\n";
	send(fd,msg,strlen(msg),0);
}
void echo_error(int fd,int errno_num)
{
	switch(errno_num){
		case 400:
			break;
		case 401:
			break;
		case 403:
			break;
		case 404:
			show_404(fd);
			break;
		case 500:
			break;
		case 501:
			break;
		case 503:
			break;
		default:
			break;
	}
}

int echo_www(int fd,const char*path,int size)
{
	int new_fd = open(path,O_RDONLY);
	if(new_fd < 0)
	{
		print_log("open file error!",FATAL);
		return 404;
	}

	const char*echo_line="HTTP/1.0 200 OK\r\n";
	send(fd,echo_line,strlen(echo_line),0);
	const char *blank_line="\r\n";
	send(fd,blank_line,strlen(blank_line),0);

	if(sendfile(fd,new_fd,NULL,size) < 0)//将两个文件拷贝 不用从内核到用户，直接在内核从文件描述符拷贝
	{
		print_log("send file error!",FATAL);
		return 200;
	}
	close(new_fd);
}
void drop_header(int fd)//丢弃报头
{
   char buff[SIZE];
   int ret = -1;
   do{
       ret = get_line(fd,buff,sizeof(buff));
   }while(ret > 0 && strcmp(buff,"\n"));
}

int exe_cgi(int fd,const char *method,\
		   const char* path,const char*query_string)
{
    int content_len = -1;
    char METHOD[SIZE/10];
    char QUERY_STRING[SIZE];
    char CONTENT_LENGTH[SIZE];
    if(strcasecmp(method,"GET")==0)
	{
		drop_header(fd);
	}else   //POST
    {
        char buff[SIZE];
        int ret = -1;
        do{
            ret = get_line(fd,buff,sizeof(buff));
            if(strncasecmp(buff,"Content-Length: ",16)==0){//表示后续有多少字节
                content_len = atoi(&buff[16]);//下标从16开始
            }
        }while(ret > 0 && strcmp(buff,"\n")); //读到换行 或者空行
        if(content_len == -1){
            echo_error(fd,401);
            return -1;
        }
    } 

    printf("cgi: path:%s\n",path);
    int input[2];
    int output[2];
    if(pipe(input) < 0)//管道
    {
        echo_error(fd,401);
        return -2;
    }
    if(pipe(output)<0)
    {
        echo_error(fd,401);
        return -3;
    }
    const char *echo_line="HTTP/1.0 200 OK\r\n";
    send(fd,echo_line,strlen(echo_line),0);
    const char *type = "Content-type:text/html;charset=ISO-8859-1\r\n";//给浏览器说明这是一个html文件 
    send(fd,type,strlen(type),0);
    const char *blank_line="\r\n";
    send(fd,blank_line,strlen(blank_line),0);

    pid_t id = fork();
	if(id < 0)
    {
		echo_error(fd,501);
	        return -2;
	}else if(id == 0)//child
    {
        close(input[1]);
        close(output[0]);
        sprintf(METHOD,"METHOD=%s",method);
        putenv(METHOD);//导出环境变量 
        if(strcasecmp(method,"GET") == 0)
        {
            sprintf(QUERY_STRING,"QUERY_STRING=%s",query_string); 
            putenv(QUERY_STRING);
        }else{
            sprintf(CONTENT_LENGTH,"CONTENT_LENGTH=%d",content_len);
            putenv(CONTENT_LENGTH);
        }
        dup2(input[0],0);//将input0 重定向到0
        dup2(output[1],1);
        execl(path,path,NULL);
        exit(1);
	}else{
	        close(input[0]);
	        close(output[1]);
	        int i = 0;
	        char c = '\0';
	        for(;i < content_len; i++) 
	        {
	            recv(fd,&c,1,0);
	            write(input[1],&c,1);
	        }
	        while(1)
	        {
	            ssize_t s = read(output[0],&c,1);
	            if(s > 0)
	            {
	                send(fd,&c,1,0);
	            }else{
	                break;
	            }
	        }

        waitpid(id,NULL,0);//阻塞式的等
        close(input[1]);
        close(output[0]);
    }
}
void* handler_request(void *arg)
{
    int fd = (int)arg;
    int errno_num = 200;
    int cgi = 0;
    char *query_string = NULL;

#ifdef _DEBUG_
    printf("########################################\n");
	char buff[SIZE];
	int ret = -1;
	do{
        ret = get_line(fd,buff,sizeof(buff));
        printf("%s",buff);
    }while(ret > 0 && strcmp(buff,"\n"));
    printf("########################################\n");
#else
    //1 method,2 url -> GET POST / url ->exist -> pri
    char method[SIZE/10];
    char url[SIZE];
    char path[SIZE];
    char buff[SIZE];
    int i,j;
    if(get_line(fd,buff,sizeof(buff)) <= 0){
        print_log("get request line error",FATAL);
        errno_num = 501;
        goto end;
    }
    i = 0,j = 0;
    while( i < sizeof(method) - 1 && j < sizeof(buff) &&\
			!isspace(buff[j])){
        method[i] = buff[j];
        i++,j++;
    }
    method[i] = 0;
    //GET  /a/b http/1.0
    while(isspace(buff[j]) && j < sizeof(buff)){
        j++;
    }
    i = 0;
    while( i < sizeof(url) && j < sizeof(buff) && \
            !isspace(buff[j])){
        url[i] = buff[j];
        i++,j++;
    }
    url[i] = 0;  //至少是一个/，若是/则将首页返回
    printf("method: %s,url: %s\n",method,url);
    if(strcasecmp(method,"GET") && strcasecmp(method,"POST")){  //忽略大小写
        print_log("method is not ok!",FATAL);
        errno_num = 501;  //方法有问题
        goto end;
    }

    if(strcasecmp(method,"POST") == 0){
        cgi = 1;
    }
	///////////url
    query_string = url;
    while(*query_string != 0){
        if(*query_string == '?'){ //路径和参数以？分隔
            cgi = 1;
            *query_string = '\0';//前半部分是路径，后半部分是参数
            query_string++;//路径找到了
            break;
        }
        query_string++;
    }
    sprintf(path,"wwwroot%s",url);// 将后面的信息输出在path里
	// /a/b/c

    if(path[strlen(path)-1] == '/'){///url有可能是一个根目录 或者以/结尾
        strcat(path,"index.html");//给用户返回一个主页
	}
    printf("path: %s\n",path);
    ///a/b/c.html
     struct stat st;
    if(stat(path,&st) < 0){ //资源不存在
        print_log("path not found!",FATAL);
        errno_num = 404;
		goto end;
	}else{
		if(S_ISDIR(st.st_mode)){//如果是目录
			strcat(path,"/index.html");//它本来没有/，就带一个
		}else{   //
			if((st.st_mode & S_IXUSR) ||\
			   (st.st_mode & S_IXGRP) ||\
			   (st.st_mode & S_IXOTH) )
				cgi = 1;
			}
		}

		//path -> cgi
		if(cgi){  //cgi合法
			exe_cgi(fd,method,path,query_string);
		}else{//正常模式
			drop_header(fd);
			errno_num = echo_www(fd,path,st.st_size);
		}
end:
	echo_error(fd,errno_num);
	close(fd);
#endif
}

