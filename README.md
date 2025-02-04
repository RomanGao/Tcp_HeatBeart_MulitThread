## TCP粘包问题
TCP粘包问题是指在TCP长连接通信过程中，由于数据包的传输特性，多个小数据包可能会被合并成一个大的数据包发送，或者一个大数据包被拆分成多个小数据包发送，导致接收端收到的数据包数量和发送端不一致的问题。这种现象在TCP协议中是正常的，因为TCP是面向流的协议，保证的是数据的有序性和可靠性，而不是数据包的边界。

为了解决TCP粘包问题，可以采取以下几种方法：

1. 使用定长包头：在发送数据时，每个数据包都包含一个固定的包头，包头中包含了包的长度信息。接收端根据包头中的长度信息来解析每个完整的数据包。

2. 包头加包体：在发送数据时，将数据分为包头和包体两部分。包头包含了包体的长度信息，接收端先读取包头，然后根据长度信息读取对应的包体。

3. 使用特殊分隔符：在发送数据时，使用特殊的分隔符来分隔不同的数据包。接收端读取数据时，遇到分隔符就认为一个数据包结束。

4. 应用层协议：定义一种应用层的协议，比如使用HTTP、JSON、Protocol Buffers等，通过协议来规定数据的格式和传输方式，以此来避免粘包问题。

> 本项目采取了**方法1**实现TCP粘包的解决方案，即每个数据包都包含一个固定的包头，包头中包含了包的长度信息。


心跳包的作用是检测和维护TCP连接的活性，确保连接不会因为长时间无数据交换而被防火墙或其他网络设备关闭（保活）。在客户端和服务端长时间不通信时，通过定期发送心跳包来维持连接状态，避免因网络问题导致的死链情况（防死链）。

## 运行：
客户端

```shell
sh server.sh
```


服务端

```shell
sh client.sh
```

学习来选：

<https://www.bilibili.com/video/BV1ar421c7e8/?spm_id_from=333.1365.list.card_archive.click&vd_source=e8d4b14c3216687a2cdfe802ffc83ad5>