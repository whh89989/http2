ROOT_PATH=$(shell pwd)#用shell的方式运行pwd命令
INCLUDE=$(ROOT_PATH)/lib/include
LIB=$(ROOT_PATH)/lib/lib
LDFLAGS=-lmysqlclient
cc=g++

.PHONY:all
all:insert_cgi select_cgi


insert_cgi:insert_cgi.o sql.o
	$(cc) -o $@ $^ -L$(LIB) $(LDFLAGS)
select_cgi:select_cgi.o sql.o 
	$(cc) -o $@ $^ -L$(LIB) $(LDFLAGS) #指定位置找库

%.o:%.cpp
	$(cc) -c $< -I$(INCLUDE)  #显式的告诉在指定目录下查找

.PHONY:clean
clean:
	rm -f *.o insert_cgi select_cgi



