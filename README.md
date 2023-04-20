# WebServer
this is a c++ project-WebServer

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
