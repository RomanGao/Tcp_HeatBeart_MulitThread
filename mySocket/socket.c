#include "socket.h"
#include<stdio.h>
#include<errno.h>

int initSocket()
{
    //AF_INET:ipv4, SOCK_STREAM:tcp 0:SOCK_STREAM的默认协议
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        perror("socket error");
        return -1;
    }
    return lfd;
}

void initSockaddr(struct sockaddr *addr, unsigned short port, const char *ip)
{
    struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
    addr_in->sin_family = AF_INET; //ipv4
    //htons:主机字节序转换为网络字节序
    addr_in->sin_port = htons(port); //端口号  //linux系统下网络字节序转换
    //inet_addr:将点分十进制的ip地址转换成网络字节序的32位整数
    addr_in->sin_addr.s_addr = inet_addr(ip);
}

int setListen(int lfd, unsigned short port)
{
    struct sockaddr addr;
    initSockaddr(&addr, port, "0.0.0.0"); //"0.0.0.0"->INADDR_ANY:监听所有地址
    
     //设置端口复用
     //文件描述符，监听级别，干什么事情，端口封闭还是打开，地址大小
    int opt = 1;//默认打开
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    //结构绑定
    int res = bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
    if(res == -1)
    {
        perror("bind error!");
        return -1;
    }
    //设置监听
    //第二个参数是一次性同时监听的数量
    res = listen(lfd, 128);
    if(res == -1)
    {
        perror("listen error!");
        return -1;
    }
    return 0;
}

int accpetConnect(int lfd, struct sockaddr *addr)
{;
    int connfd = -1;
    if(addr == NULL)
    {//不关心客户端的地址，只关心客户端的socket文件描述符
        connfd = accept(lfd, NULL, NULL);
    }
    else
    {
        socklen_t len = sizeof(struct sockaddr_in);
        connfd = accept(lfd, addr, &len);
    }
    if(connfd == -1)
    {
        perror("accpet error");
        return -1;
    }
    return connfd;
}

int connectToHost(int fd, const char *ip, unsigned short port)
{
    struct sockaddr_in addr;
    initSockaddr((struct sockaddr *)&addr, port, ip);
    int ret = connect(fd, (struct sockaddr *)(&addr), sizeof(addr));
    if(ret == -1)
    {
        perror("connect error");
        return -1;
    }
    return 0;
}
//发送固定长度的数据，返回实际发送的字节数
int writen(int fd, const char *buffer, int len)
{
    int left = len;
    int sendlen = 0;
    const char* ptr = buffer;
    while(left>0)
    {
        //不停的数据发送
        //sendlen = send(fd, ptr, left, 0); //一样
        sendlen = write(fd, ptr, left);
        if(sendlen <=0) //发送被信号中断
        {
            if(errno == EINTR) //Interrupted system call 
            {
                sendlen = 0; //重发
            }
            else
            {
                perror("send data error");
                return -1;
            }
        }
        ptr += sendlen;
        left -= sendlen;
    }
    return len;  //发送的字节数
}
//接收固定长度的数据，返回实际接收的字节数
int readn(int fd, char *buffer, int len)
{
    int left = len; 
    int recvlen = 0;
    char* ptr = buffer;
    while(left>0)
    {
        //不停的接收数据
        //recvlen = recv(fd, ptr, left, 0); //一样
        recvlen = read(fd, ptr, left);
        if(recvlen == -1) //接收被信号中断
        {
            if(errno == EINTR) //Interrupted system call 
            {
                recvlen = 0; //重发
            }
            else
            {
                perror("recv data error\n");
                return -1;
            }
        }
        else if(recvlen == 0)
        {
            printf("对方主动断开了连接...\n");
            return -1;
        }
        ptr += recvlen;
        left -= recvlen;
    }
    //printf("readn len:%d left:%d\n",len, left);
    return len - left;  //实际接收的字节数
}

int recvMessage(int fd, char **buffer, enum Type *t)
{
    int dataLen = 0;
    //数据包头
    int ret = readn(fd, (char*)&dataLen, sizeof(int));
    if(ret == -1)
    {
        *buffer = NULL;
        return -1;
    }
    dataLen = ntohl(dataLen); //网络字节序转换,大端转小段
    //数据类型
    char ch;
    readn(fd, &ch, 1);
    *t = ch == 'H'?HEART:MESSAGE;
    //数据大小,数据长度应该为datalen-1，没有-1是为了存‘\0'
    char* tmpbuf = (char*)calloc(dataLen, sizeof(char));
    if(tmpbuf == NULL)
    {
        //内存申请失败
        *buffer = NULL;
        return -1;
    }
    ret = readn(fd, tmpbuf, dataLen-1);
    if(ret != dataLen -1)
    {
        free(tmpbuf);
        *buffer = NULL;
        return -1;
    }
  //  printf("recvmsg: ret :%d dataLen:%d\n", ret, dataLen);
    *buffer = tmpbuf;
    return ret;
}

bool sendMessage(int fd, const char *buffer, int len, enum Type t)
{
    //      数据长度 + 数据类型长度 + 4个字节的数据头
    int dataLen = len + 1 + sizeof(int); //加上类型字节
    //申请内存堆
    char* data = (char*)malloc(dataLen);
    if(data == NULL)
    {
        return false;
    }
    //写入数据长度
    int netLen = htonl(len + 1); //网络字节序转换
    memcpy(data, &netLen, sizeof(int));
    //写入数据类型
    char* ch = t == HEART ? "H":"M";
    memcpy(data + sizeof(int), ch, 1);
    //写入数据
    memcpy(data + sizeof(int) + 1, buffer, len);
    int ret = writen(fd, data, dataLen);
    return ret == dataLen;
    free(data);
    //发送数据
    return false;
}
