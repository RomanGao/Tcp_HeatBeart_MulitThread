#include<stdio.h>
#include"socket.h"
#include<string.h>
#include"clientList.h"

pthread_mutex_t mutex;  //定义互斥锁

struct shareInfo
{
    int fd;
    int count;  //客户端和服务器没收到心跳包次数统计
};
void * parseRecvMessage(void *arg)
{
    struct ClientInfo* info = (struct ClientInfo*)arg; //最前面的*是用于解引用
    //接收数据
    while(1)
    {
        char* buffer;
        enum Type t;
        int len = recvMessage(info->fd, &buffer, &t);
        printf("parse len:%d\n",len);
        if(buffer == NULL)
        {
            //接收数据失败，子线程退出
            printf("fd = %d, recvMessage failed, exit\n", info->fd);
            pthread_exit(NULL);
        }
        else
        {
            if(t == HEART)
            {
                printf("xin tiao bao heart: %s - len: %d\n", buffer, len);
                pthread_mutex_lock(&mutex);
                info->count =0;
                pthread_mutex_unlock(&mutex);
                //收到心跳包需要回复数据
                sendMessage(info->fd, buffer, len, HEART);
            }
            else
            {
                const char* str = "server to client :shu ju bao";
                //普通的数据包
                printf("shu ju bao message: %s \n",buffer);
                sendMessage(info->fd, str, strlen(str), MESSAGE);  
            }
            free(buffer);
        }
    }

    return NULL;
}

//1、发送心跳包数据
//2、检测心跳包，查看接收到的数据是否正常
void* HeartBeat(void *arg)
{
    struct ClientInfo* head = (struct ClientInfo*)arg; //最前面的*是用于解引用
    struct ClientInfo* info = NULL;

    while(1)
    {
        info = head->next;
        while(info)
        {
            pthread_mutex_lock(&mutex);
            info->count++; //默认没有收到心跳包数据
            printf("server heart count = %d\n", info->count);
            if(info->count >5)
            {
                //客户端和服务端断开连接
                printf("clinet duan kai fd = %d\n", info->fd);
                close(info->fd);
                //释放套接字资源，退出客户端程序
                //在子线程中，需要将接收该数据对应的子线程终止
                pthread_cancel(info->pid); //子线程退出
                removeClient(head, info->fd); //从链表中删除该客户端信息
            }
            pthread_mutex_unlock(&mutex);
            info = info->next;
        }
        //sendMessage(info->fd, "xin tiao bao", strlen("xin tiao bao"), HEART);
        sleep(3);
    }
}
int main()
{
    unsigned short port = 8888;
    //const char* ip = "127.0.0.1";
    int fd = initSocket();
    //设置监听和绑定
    setListen(fd, port);
    //创建链表
    struct ClientInfo* client = createClientList();

    pthread_mutex_init(&mutex, NULL);
    
    //添加心跳包子线程
    pthread_t pid1;
    pthread_create(&pid1, NULL,HeartBeat, client); //头结点
    // send data
    while(1)
    {
        int sockfd = accpetConnect(fd, NULL);
        if(sockfd == -1)
            continue;
        //添加到链表
        struct ClientInfo* node =prependClient(client, sockfd);
        //创建子线程，处理数据
        //pid, 线程熟悉（NULL为默认属性），回掉函数（线程的处理动作），回调函数的参数
        pthread_create(&node->pid, NULL,parseRecvMessage,node);
        //主线程与子线程分离
        pthread_detach(node->pid); //子线程退出后，父线程不等待子线程退出
    }
    pthread_join(pid1, NULL);
    close(fd);
    pthread_mutex_destroy(&mutex);
    return 0;
}