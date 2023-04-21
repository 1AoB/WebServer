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
CXX = g++    # C++编译器的名称
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

---

## Makefile的语法
```bash
1. 变量名=变量值
eg:var=hello
#获取变量的值 $(变量名)
$(var)
2. 预定义变量
AR:归档维护程序的名称 , 默认值为ar
CC:C编译器的名称,默认是cc
CXX:C++编译器的名称,默认是g++
$@:目标的完整名称
$<:第一个依赖文件的名称
$^:所有依赖文件

eg: 
app:main.c a.c b.c #要生成的文件:需要的依赖
    gcc -c main.c a.c b.c -o app #如果这些依赖文件,在文件夹中存在,就使用这些依赖执行这行命令
#使用变量修改后为:
app:main.c a.c b.c
    $(CC) -c &^ -o $@

3. 模式匹配
%.o:%c
    %:通配符,只匹配一个字符串
    两个%匹配的是同一个字符串
eg:
add.o:add.c #要生成的文件:需要的依赖
    gcc -c add.c -o add.o #如果这些依赖文件,在文件夹中存在,就使用这些依赖执行这行命令
div.o:div.c #要生成的文件:需要的依赖
    gcc -c div.c -o div.o #如果这些依赖文件,在文件夹中存在,就使用这些依赖执行这行命令
mult.o:mult.c #要生成的文件:需要的依赖
    gcc -c mult.c -o mult.o #如果这些依赖文件,在文件夹中存在,就使用这些依赖执行这行命令
main.o:main.c #要生成的文件:需要的依赖
    gcc -c main.c -o main.o #如果这些依赖文件,在文件夹中存在,就使用这些依赖执行这行命令
# 优化后:
%.o:%c
    gcc -c &< -o &@
```
---

# 项目运行

在执行bin/server命令之后,在浏览器中打开http://192.168.8.128:8989

http://192.168.8.128 是你在ubuntu中的服务器ip,8989是你在项目代码的timer文件夹下的main.cpp中,设置的端口号
