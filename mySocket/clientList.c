#include "clientList.h"
#include<stdio.h>

struct ClientInfo *createClientList()
{
    //带头部节点的链表，头部不存储数据
    struct ClientInfo *head = (struct ClientInfo *)malloc(sizeof(struct ClientInfo));
    head->next = NULL;
    if(head == NULL)
        return NULL;
    return head;
}

struct ClientInfo *prependClient(struct ClientInfo *head, int fd)
{
    //头插法，头不存储数据
    struct ClientInfo *newClient = (struct ClientInfo *)malloc(sizeof(struct ClientInfo));
    newClient->fd = fd;
    newClient->next = head->next;
    head->next = newClient;
    return newClient; //返回当前节点的地址
}

bool removeClient(struct ClientInfo *head, int fd)
{
    struct ClientInfo *current = head->next;
    struct ClientInfo *previous = head;
    while(current!= NULL)
    {
        if(current->fd == fd)
        {
            previous->next = current->next;
            free(current);
            printf("remove client fd = %d\n", fd);
            return true;
        }
        previous = current;
        current = current->next;
    }
    printf("client fd = %d not found\n", fd);
    return false;
}

void destroyClientList(struct ClientInfo *head)
{
    //连同head一起删掉
    struct ClientInfo *current = head;
    struct ClientInfo *temp = NULL;
    while(current!= NULL)
    {
        temp = current;
        current = current->next;
        free(temp);
    }
}
