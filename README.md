# WebServer
this is a c++ project-WebServer

## 环境要求

- Linux
- C++14
- MySql

## 功能

- 利用单例模式与阻塞队列实现异步的日志系统，记录服务器运行状态；

## MYSQL
linux中安装mysql:https://segmentfault.com/a/1190000023081074

mysql的操作:

mysql -u debian-sys-maint -p        // 用户名以自己的配置文件为准  登录mysql

show databases;

----

**更改密码后**登录mysql:

mysql -u root -p

再输入自己设置的mysql密码即可登录mysql

### 在mysql中项目启动
```sql
// 建立yourdb库
create database webserver;

//进入webserver库
use webserver;

// 创建user表
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

//查看自己创建的表
show tables;

// 添加数据
INSERT INTO user(username, password) VALUES('wxn', 'wxn');

//查看user表中的数据
select * from user;
```
# Makefile(通过make命令找到当前路径下的Makefile文件,执行Makefile文件中的命令)
**特别注意:一定要再WebServer目录下执行bin/server才可正常运行,直接在bin目录下运行,访问会404**
```bash
all:
        mkdir -p bin  #生成bin目录
        cd build && make # 进入到build目录中,执行make命令
        #在webserver目录下执行make命令后,会在当前目录下生成bin目录,bin目录下有一个server可执行文件
```
bulid目录下的Makefile文件:
```bash
CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = server
OBJS = ../code/log/*.cpp ../code/pool/*.cpp ../code/timer/*.cpp \
       ../code/http/*.cpp ../code/server/*.cpp \
       ../code/buffer/*.cpp ../code/main.cpp

all: $(OBJS)
        $(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

clean:
        rm -rf ../bin/$(OBJS) $(TARGET)
```
