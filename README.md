# WebServer
this is a c++ project-WebServer

# 项目描述
- 采用epoll+Reactor模式去实现的高并发服务器 ;
>(注:
>
>Reactor是快递到了快递站,然后发消息让你自己去取.
>
>Proactor是快递到了并且送货上门,不用你自己去取;
>
>Reactor是需要你自己去读数据,然后才能处理读到的事件.
>
>Proactor是已经帮你读好了,你直接去处理事件就行了)

- 利用标准库容器封装char，实现自动增长的缓冲区;
- 基于小根堆实现的定时器，关闭超时的非活动连接;
>比如说:在60s内,你没有任何操作,那就给你关闭掉
>
>小根堆可以保证是最小的时间

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
#CXX,CFLAGS,TARGET,OBJS是是个变量
CXX = g++    # C++编译器的名称
CFLAGS = -std=c++14 -O2 -Wall -g # 表示用于 C 编译器的选项

TARGET = server # 最终生成的目标文件
OBJS = ../code/log/*.cpp ../code/pool/*.cpp ../code/timer/*.cpp \ # 列出所以依赖的文件
       ../code/http/*.cpp ../code/server/*.cpp \
       ../code/buffer/*.cpp ../code/main.cpp

all: $(OBJS)
        $(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

clean:
        rm -rf ../bin/$(OBJS) $(TARGET) # 当运行make clean命令时,它会删除../bin/$(OBJS)这条路径中列出来的文件和$(TARGET)可执行文件
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

4. 函数
1)$(wildcard pattern)#表示在pattern路径下查找:某种后缀的文件
eg: $(wildcard ./*.c)#表示在当前文件夹下寻找.c文件,并返回,返回的格式为:a.c,b.c,c.c,d.c等等
2)$(patsubst pattern,replacement,text)
#pattern表示要搜索的字符串模式,replacement表示要替换为的字符串,text是要进行替换的文本
eg:$(pastsubst %.c,%.o,x.c bar.c)
#表示将x.c bar.c中以.c为结尾的文件,转换为以.o为结尾的文件,所以:返回的格式为x.o bar.o

5.clean的用法
eg:
clean:
    rm -f $(OBJ_FILES) myapp
解释:其中，rm -f命令用于强制删除文件或目录，$(OBJ_FILES)变量包含需要删除的所有目标文件路径，myapp是可执行文件名。
当我们运行make clean命令时，它将会删除所有在$(OBJ_FILES)变量中列出的文件和myapp可执行文件。
```
---

# GDB调试
eg: gcc test.c -o test -g
1. gdb test # 进入调试,test是可执行的具有调试信息的文件
2. q # 退出调试
3. ctrl+l # 可以清屏
4. l/list select.cpp:8 # 查看select.cpp的第8行左右的代码
5. b 8 # 8是行号,表示:在第8行打上一个断点
   b select.cpp:8 # 表示在select.cpp的第8行打上一个断点(因为一个大的c++项目会有更多分文件)
6. i b # 表示: information break -> 查看断点信息
7. run # 开启调试
8. s/step # 表示向下单步调试
   finish # 表示跳出函数体
9. next # 表示向下执行一句,且直接就不进入函数体
10. p/print 变量名 # 打印变量值
    ptype 变量名 # 打印变量的类型
11. c/continue # 继续运行,直到下一个断点才停下 

# 项目运行

在执行bin/server命令之后,在浏览器中打开http://192.168.8.128:8989



http://192.168.8.128 是你在ubuntu中的服务器ip,8989是你在项目代码的code文件夹下的main.cpp中,设置的端口号


![image](https://user-images.githubusercontent.com/78208268/233582058-cdaefda5-2d39-418a-8d54-e749c98db782.png)

# 压力测试
```bash
cd webbench-1.5/
make
./webbench -c 2000 -t 5 http://192.168.8.128:8989/
```
![image](https://user-images.githubusercontent.com/78208268/236235713-4a971851-3700-4dd8-863e-a8753e12a3ea.png)

