//用于服务端存储客户端的链表
#pragma once
#include <stdbool.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include<string.h>

//定义链表的节点
struct ClientInfo
{
    int fd;  //通信
    int count;  //计数器
    pthread_t pid;  //进程号
    struct ClientInfo* next;  //指向下一个节点
};

//创建一个链表，带头结点,返回头结点
struct ClientInfo* createClientList();
//添加一个节点（头插法）
struct ClientInfo* prependClient(struct ClientInfo* head, int fd);
//删除一个节点
bool removeClient(struct ClientInfo* head, int fd);
//销毁链表，头指针也删除
void destroyClientList(struct ClientInfo* head);