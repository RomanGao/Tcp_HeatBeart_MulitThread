#pragma once
#include<stdbool.h>
#include<arpa/inet.h>  //linux的头文件
#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

//数据包类型
enum Type {HEART, MESSAGE}; //心跳包和普通数据
 //               "H"    "M"
//数据包格式：数据长度 + 数据包类型 + 数据块
//            int      char     char*
//             4        1         n
//1、初始化一个套接字
int initSocket();  //返回一个通于通信文件描述符
//初始化结构体函数
void initSockaddr(struct sockaddr* addr, unsigned short port, const char*ip);
//设置监听
int setListen(int lfd, unsigned short port);

//接收客户端连接
int accpetConnect(int lfd, struct sockaddr* addr);

//---------------------------------
//客户端，不需要使用代码绑定端口
//连接服务器
int connectToHost(int fd, const char* ip, unsigned short port);
//发送 固定 字节数的数据,发送固定字节数的函数
int writen(int fd, const char* buffer, int len);
//接收 固定 字节数数据，基于循环处理，可能回连续读多次
int readn(int fd, char* buffer, int len);

//添加两个函数，解决粘包问题
//发送数据包, data:原始数据
bool sendMessage(int fd, const char* data, int len, enum Type t);
//接收数据包,buffer大小拿不准，那就不写，让函数开发者给出内存大小
//将一级指针改成二级， 返回接收的报文长度
int recvMessage(int fd, char** buffer, enum Type* t);
