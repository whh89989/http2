#include "sql.h"
using namespace std;

sql::sql(const string &_user,\
		 const string &_ip,\
		 const string &_passwd,\
		 const string &_db,\
		 const int &_port):user(_user),ip(_ip),passwd(_passwd),\
						  db(_db),port(_port)
{
	conn = mysql_init(NULL);
}

int sql::connect()
{
	if(mysql_real_connect(conn,ip.c_str(),user.c_str(),\
				passwd.c_str(),db.c_str(),port,NULL,0)){
		cout<<"connect success!!!"<<endl;
		return 0;
	}else{
		cout<<"connect fail..."<<endl;
		return 1;
	}
}


int sql::insert(const string &name,\
		        const string &sex,\
				const string &score)
{
	//"insert into"
	string sql = "INSERT INTO scores(id,name, sex,score) values (NULL,'";
	sql+=name;
	sql+="','";
	sql+=sex;
	sql+="','";
	sql+=score;
	sql+="')";
	cout<<sql<<endl;

	int ret = mysql_query(conn,sql.c_str());
	cout<<"ret:"<<ret<<endl;
	if(ret == 0)
	{
		cout<<"insert success"<<endl;
		std::string sql1 = "select max(id) from scores";
	    ret = mysql_query(conn,sql1.c_str());
		if(ret == 0){
			MYSQL_RES *res = mysql_store_result(conn);
			int cols = mysql_num_fields(res);
			//std::cout<<"lines: "<<lines<<" cols: "<<cols<<std::endl;

			std::cout<<"your id is: "<<endl;
			MYSQL_ROW row = mysql_fetch_row(res);
			for(int j = 0;j < cols; j++){
				std::cout<<row[j]<<' ';
			}
		}
	}
}
int sql::select_one(const string &id)
{
	std::string sql = "select * from scores where id=";
	sql += id;
	sql+=";";
	cout<<sql<<endl;

	int ret = mysql_query(conn,sql.c_str());
	if(ret == 0){
		MYSQL_RES *res = mysql_store_result(conn);
    	if(res){
			int lines = mysql_num_rows(res);
			int cols = mysql_num_fields(res);
			MYSQL_FIELD *fd = NULL;
			for(;fd = mysql_fetch_field(res) ;){
				std::cout<<fd->name<<' ';//拿出列名
			}
			std::cout<<endl;
			int i = 0;
			for(;i < lines; i++){
				MYSQL_ROW row = mysql_fetch_row(res);
				for(int j = 0;j < cols; j++){
					std::cout<<row[j]<<' ';
				}
				std::cout<<std::endl;
			}
		}
		return 0;
	}
	return -1;
}
int sql::select_all()
{
	std::string sql = "select * from scores";
	int ret = mysql_query(conn,sql.c_str());
	if(ret == 0){
		MYSQL_RES *res = mysql_store_result(conn);
    	if(res){
			int lines = mysql_num_rows(res);
			int cols = mysql_num_fields(res);
			std::cout<<"lines: "<<lines<<" cols: "<<cols<<std::endl;
			MYSQL_FIELD *fd = NULL;
			for(;fd = mysql_fetch_field(res) ;){
				std::cout<<fd->name<<' ';//拿出列名
			}
			std::cout<<endl;
			int i = 0;
			for(;i < lines; i++){
				MYSQL_ROW row = mysql_fetch_row(res);
				for(int j = 0;j < cols; j++){
					std::cout<<row[j]<<' ';
				}
				std::cout<<std::endl;
			}
		}
		return 0;
	}
	return -1;
}
sql::~sql()
{
	mysql_close(conn);
}
