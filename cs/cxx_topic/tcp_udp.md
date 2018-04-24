# TCP
Note that TCP does not guarantee that the data will be received by the other endpoint, as this is impossible. It delivers data to the other endpoint if possible, and notifies the user (by giving up on retransmissions and breaking the connection) if it is not possible. Therefore, TCP cannot be described as a 100% reliable protocol; it provides **reliable delivery of data or reliable notification of failure.**

**socket中TCP的三次握手建立连接**  
![socket中TCP的三次握手建立连接详解](http://images.cnblogs.com/cnblogs_com/skynet/201012/201012122157467258.png)  
从图中可以看出:

1. 当客户端调用connect时,触发了连接请求,向服务器发送了SYN J包,这时connect进入阻塞状态.  
1. 服务器监听到连接请求,即收到SYN J包,调用accept函数接收请求向客户端发送SYN K ,ACK J+1,这时accept进入阻塞状态.  
1. 客户端收到服务器的SYN K ,ACK J+1之后,这时connect返回,并对SYN K进行确认,服务器收到ACK K+1时,accept返回,至此三次握手完毕,连接建立.

客户端的`connect`在三次握手的第二个次返回,而服务器端的`accept`在三次握手的第三次返回.

内核为任何一个给定的监听套接字维护两个队列:

1. **未完成连接队列(incomplete connection queue)**: 每个这样的SYN 分节对应其中一项:
已由某个客户发出并到达服务器,而服务器正在等待完成相应的TCP三路握手过程.这些套接字处于SYN_RCVD状态
2. **已完成连接队列(completed connection queue)**, 每个已完成TCP 三路握手过程的客户对应其中一项. 这些套接字处于established 状态

当来自客户的SYN到达时,TCP在未完成连接队列中创建一个新项.  
如果三路握手正常完成, 该项就从未完成连接队列移到已完成连接队列的末尾.  
当进程调用accept时,已完成连接队列中的队头项将返回给进程,或者如果该队列为空,那么进程将被投入睡眠,直到TCP 在该队列中放入一项才唤醒它

**TCP FLAG 标记**  
基于标记的TCP包匹配经常被用于过滤试图打开新连接的TCP数据包.  
TCP标记和他们的意义如下所列:

* F : FIN - 结束; 结束会话
* S : SYN - 同步; 表示开始会话请求
* R : RST - 复位;中断一个连接
* P : PUSH - 推送; 数据包立即发送
* A : ACK - 应答
* U : URG - 紧急
* E : ECE - 显式拥塞提醒回应
* W : CWR - 拥塞窗口减少
 
**长连接的情况下出现了不同程度的延时**  
在一些长连接的条件下, 发送一个小的数据包, 结果会发现从数据write成功到接收端需要等待一定的时间后才能接收到, 而改成短连接这个现象就消失了
(如果没有消失,那么可能网络本身确实存在延时的问题,特别是跨机房的情况下).  
在长连接的处理中出现了延时, 而且时间固定,基本都是40ms, 出现40ms延时最大的可能就是由于没有设置`TCP_NODELAY`.  
在长连接的交互中,有些时候一个发送的数据包非常的小,加上一个数据包的头部就会导致浪费,而且由于传输的数据多了,就可能会造成网络拥塞的情况,
在系统底层默认采用了Nagle算法,可以把连续发送的多个小包组装为一个更大的数据包然后再进行发送.
但是对于我们交互性的应用程序意义就不大了,在这种情况下我们发送一个小数据包的请求,就会立刻进行等待,不会还有后面的数据包一起发送,
这个时候Nagle算法就会产生负作用,在我们的环境下会产生40ms的延时,这样就会导致客户端的处理等待时间过长, 导致程序压力无法上去.
在代码中无论是服务端还是客户端都是建议设置这个选项,避免某一端造成延时.所以对于长连接的情况我们建议都需要设置`TCP_NODELAY`.  
对于服务端程序而言, 采用的模式一般是bind -> listen -> accept, 这个时候accept出来的句柄的各项属性其实是从listen的句柄中继承,
所以对于多数服务端程序只需要对于listen进行监听的句柄设置一次`TCP_NODELAY`就可以了,不需要每次都accept一次.

# UDP
We also say that UDP provides a connectionless service, as there need not be any long-term relationship between a UDP client and server. 
For example, a UDP client can create a socket and send a datagram to a given server and then immediately send another datagram on the same socket to a different server. 
Similarly, a UDP server can receive several datagrams on a single UDP socket, each from a different client.

![UDP客户/服务器程序所用的套接字函数](http://pic002.cnblogs.com/images/2012/367190/2012081121141279.jpg)  
如上图所示, 客户不与服务器建立连接, 而是只管使用`sendto`函数给服务器发送数据报, 其中必须指定目的地(即服务器)第地址作为参数. 
类似的, 服务器不接受来自客户的连接, 而是只管调用`recvfrom` 函数, 等待来自某个客户的数据到达.
recvfrom将接收到的数据与client 的地址一并返回, 因此服务器可以把响应发送给正确的客户.

写一个长度为0 的数据报是可行的. 在UDP情况下, 这会形成一个只包含一个IP首部和一个UDP首部而没有数据的IP数据报. 这也意味着对于UDP协议, recvfrom返回0 值是可接受的.
他并不像TCP套接字上read 返回0值那样表示对端已关闭连接. 既然UDP是无连接的, 因此也没有诸如关闭一个UDP连接之类的事情.

大多数TCP服务器是并发的, 而大多数UDP服务器是迭代的
