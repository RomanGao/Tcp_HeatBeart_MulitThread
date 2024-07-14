#include<stdio.h>
#include"socket.h"
#include<string.h>
#include<pthread.h>

pthread_mutex_t mutex;  //定义互斥锁

struct shareInfo
{
    int fd;
    int count;  //客户端和服务器没收到心跳包次数统计
};
void * parseRecvMessage(void *arg)
{
    struct shareInfo* info = (struct shareInfo*)arg; //最前面的*是用于解引用
    //接收数据
    while(1)
    {
        char* buffer;
        enum Type t;
        int len = recvMessage(info->fd, &buffer, &t);
        if(buffer == NULL)
        {
            //接收数据失败，尝试接收
            continue;
        }
        else
        {
            if(t == HEART)
            {
                printf("xin tiao bao heart: %s\n", buffer);
                pthread_mutex_lock(&mutex);
                info->count =0;
                pthread_mutex_unlock(&mutex);
            }
            else
            {
                //普通的数据包
                printf("shu ju bao message: %s \n",buffer);
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
    struct shareInfo* info = (struct shareInfo*)arg;
    while(1)
    {
        pthread_mutex_lock(&mutex);
        info->count++; //默认没有收到心跳包数据
        printf("head count %d \n", info->count);
        if(info->count >5)
        {
            //客户端和服务端断开连接
            printf("yu server duan kai %d\n", info->fd);
            close(info->fd);
            //释放套接字资源，退出客户端程序
            exit(0); //子线程退出了
        }
        pthread_mutex_unlock(&mutex);
        sendMessage(info->fd, "xin tiao bao", strlen("xin tiao bao"), HEART);
        sleep(3);
    }
}
int main()
{
    struct shareInfo info;
    unsigned short port = 8888;
    const char* ip = "127.0.0.1";

    info.fd = initSocket();
    info.count =0;
    connectToHost(info.fd, ip, port);

    pthread_mutex_init(&mutex, NULL);
    
    //创建接收数据的子线程
    pthread_t pid;
    //pid, 线程熟悉（NULL为默认属性），回掉函数（线程的处理动作），回调函数的参数
    pthread_create(&pid, NULL,parseRecvMessage,&info);

    //添加心跳包子线程
    pthread_t pid1;
    pthread_create(&pid1, NULL,HeartBeat,&info);
    // send data
    while(1)
    {
        const char* data = "ni hao a...!";
        //发送数据
        sendMessage(info.fd, data, strlen(data), MESSAGE);
        sleep(2);
    }
    pthread_join(pid, NULL);
    pthread_join(pid1, NULL);
    pthread_mutex_destroy(&mutex);
    return 0;
}