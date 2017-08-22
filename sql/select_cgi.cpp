#include "sql.h"

using namespace std;

void select_data(char *buf)
{
	
	sql obj("root","127.0.0.1","","http",3306);
	obj.connect();
	obj.select();
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

			return 1;
		}
		strcpy(buff,arg_string);
	}else if(method && strcasecmp(method, "POST") == 0){
		content_len = getenv("CONTENT_LENGTH");
		if(!content_len){

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

		cout<<"method : "<<method<<endl;
		cout<<"method error"<<endl;
		return 1;
	}

	//char* lib_path = "LD_LIBRARY_PATH=/home/whh/code/projects/http/sql/lib/lib";
	//putenv(lib_path);
	select_data(buff);
	return 0;
}
