###简介：

TeamTalk是一套开源的企业办公即时通讯软件，作为整套系统的组成部分之一，TTServer为TeamTalk 客户端提供用户登录，消息转发及存储等基础服务。

TTServer主要包含了以下几种服务器:

- LoginServer (C++): 登录服务器，分配一个负载小的MsgServer给客户端使用
- MsgServer (C++):  消息服务器，提供客户端大部分信令处理功能，包括私人聊天、群组聊天等
- RouteServer (C++):  路由服务器，为登录在不同MsgServer的用户提供消息转发功能
- FileServer (C++): 文件服务器，提供客户端之间得文件传输服务，支持在线以及离线文件传输
- MsfsServer (C++): 图片存储服务器，提供头像，图片传输中的图片存储服务
- DBProxy (JAVA): 数据库代理服务器，提供mysql以及redis的访问服务，屏蔽其他服务器与mysql与redis的直接交互


###当前支持的功能点：

- 私人聊天
- 群组聊天
- 文件传输
- 多点登录
- 组织架构设置.


###系统结构图

![](https://raw.githubusercontent.com/mogutt/TTServer/master/docs/pics/server.jpg)


###后续可考虑的功能

- 协议加密
- 手机推送
- 其他合理的酷炫功能点


###C++编译
- 整体编译:可以运行src/目录下的build.sh脚本,例如: ./build.sh version 0.0.1
- 单个模块编译:进入各自的目录,然后执行make即可,注意:base模块需要优先编译

###C++使用
- 程序启动请使用run.sh脚本,例如: ./run.sh start
- 程序重启请使用restart.sh脚本,例如: ./restart.sh msg_server

###C++部署方案
- 部署方案详见https://github.com/mogutt/TTAutoDeploy 之IM_SERVER模块


###java编译
— 编译整个项目可以运行与src同目录的packageproduct.sh, sh packageproduct.sh

###java使用
— 程序启动可以运行与src同目录的startup.sh, sh startup.sh 10400(其中10400为绑定的端口号)

###java部署方案
- 部署方案详见https://github.com/mogutt/TTAutoDeploy 之IM_SERVER模块
