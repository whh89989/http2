#include <string.h>
#include <stdlib.h>
#include <stdio.h>



void math_cal(char buff[])
{
	char *argv[3];
	char *start = buff;
	int i = 0;
	while(*start){
		if(*start == '='){
			start++;
			argv[i++] = start;
			continue;
		}
		if(*start == '&'){
			*start = '\0';
		}
		start++;
	}
	argv[i] = NULL;

	int data1 = atoi(argv[0]);
	int data2 = atoi(argv[1]);

	printf("<html>\n");
	printf("<body>");
	printf("<h1>");
	printf("%d + %d = %d\n",data1,data2,data1+data2);
	printf("</h1>\n");
	printf("<h1>\n");
	printf("%d - %d = %d\n",data1,data2,data1-data2);
	printf("</h1>");
	printf("<h1>");
	printf("%d * %d = %d\n",data1,data2,data1*data2);
	printf("</h1>");
	printf("<h1>");
	printf("%d / %d = %d\n",data1,data2,data1/data2);
	printf("</h1>");
	printf("</body>");
	printf("</html>\n");
}




int main()
{
	char *method = NULL;
	char * arg_string = NULL;
	char *content_len = NULL;
	char buff[1024];
	method = getenv("METHOD");
	if(method && strcasecmp(method,"GET") == 0){
		arg_string = getenv("QUERY_STRING");
		if(!arg_string){
			printf("get me    GET arg error\n");
			return 1;
		}
		strcpy(buff,arg_string);
	}else if(method && strcasecmp(method, "POST") == 0){
		content_len = getenv("CONTENT_LENGTH");
		if(!content_len){
			printf("get method POST content_length error!\n");
			return 2;
		}
		int i = 0;
		int c = 0;
		int nums = atoi(content_len);
		for(;i < nums;i++ ){
			read(0,&c,1);
			buff[i] = c;
		}
		buff[i] = '\0';
	}else{
		printf("get method erroe!\n");
		return 1;
	}

	math_cal(buff);
	return 0;
}
