[cpu三大架构 numa smp mpp](https://www.jianshu.com/p/81233f3c2c14)

# smp
SMP(Symmetric Multiprocessing), 对称多处理器. 顾名思义, 在SMP中所有的处理器都是对等的, 它们通过总线连接共享同一块物理内存,这也就导致了系统中所有资源(CPU,内存,I/O等)都是共享的,
当我们打开服务器的背板盖,如果发现有多个cpu的槽位,但是却连接到同一个内存插槽的位置,那一般就是smp架构的服务器,
日常中常见的pc啊,笔记本啊,手机还有一些老的服务器都是这个架构,其架构简单,但是拓展性能非常差,
从linux 上也能看到:
ls /sys/devices/system/node/# 如果只看到一个node0 那就是smp架构
可以看到只有仅仅一个node,经过大神们的测试发现,2至4个CPU比较适合smp架构.

# NUMA
NUMA(Non-Uniform Memory Access), 非均匀访问存储模型,这种模型的是为了解决smp扩容性很差而提出的技术方案,
如果说smp 相当于多个cpu 连接一个内存池导致请求经常发生冲突的话,numa 就是将cpu的资源分开,以node 为单位进行切割,每个node 里有着独有的core, memory 等资源, 这也就导致了cpu在性能使用上的提升,
但是同样存在问题就是2个node 之间的资源交互非常慢,当cpu增多的情况下,性能提升的幅度并不是很高.所以可以看到很多明明有很多core的服务器却只有2个node区.

[NUMA架构的CPU -- 你真的用好了么？](http://cenalulu.github.io/linux/numa)

NUMA中，虽然内存直接attach在CPU上，但是由于内存被平均分配在了各个die上。
只有当CPU访问自身直接attach内存对应的物理地址时，才会有较短的响应时间（后称Local Access）。
而如果需要访问其他CPU attach的内存的数据时，就需要通过inter-connect通道访问，响应时间就相比之前变慢了（后称Remote Access）。
所以NUMA（Non-Uniform Memory Access）就此得名。

下面这个命令可以很简单的看出cpu的架构以及配置
```
# lscpu
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                8
On-line CPU(s) list:   0-7
Thread(s) per core:    1 #每个core 有几个线程
Core(s) per socket:    4 #每个槽位有4个core
Socket(s):             2 #服务器面板上有2个cpu 槽位
NUMA node(s):          2 #nodes的数量
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 44
Stepping:              2
CPU MHz:               2128.025
BogoMIPS:              4256.03
Virtualization:        VT-x
L1d cache:             32K
L1i cache:             32K
L2 cache:              256K
L3 cache:              8192K
NUMA node0 CPU(s):     0,2,4,6 #对应的core
NUMA node1 CPU(s):     1,3,5,7
```

由于numa 架构经常会有内存分配不均匀的情况,常常需要手动干预,除了代码以外,还有linux命令进行cpu的绑定:
```bash
taskset  -cp 1,2  25718 #将进程ID 25718 绑定到cpu的第1和第2个core 里
```

假设你是Linux教父Linus，对于NUMA架构你会做哪些优化？下面这点是显而易见的：
既然CPU只有在Local-Access时响应时间才能有保障，那么我们就尽量把该CPU所要的数据集中在他local的内存中就OK啦.
没错，事实上Linux识别到NUMA架构后，默认的内存分配方案就是：优先尝试在请求线程当前所处的CPU的Local内存上分配空间。如果local内存不足，优先淘汰local内存中无用的Page（Inactive，Unmapped）。

# MPP
MPP (Massive Parallel Processing) ,这个其实可以理解为刀片服务器,每个刀扇里的都是一台独立的smp架构服务器,且每个刀扇之间均有高性能的网络设备进行交互,保证了smp服务器之间的数据传输性能.
相比numa 来说更适合大规模的计算,唯一不足的是,当其中的smp 节点增多的情况下,与之对应的计算管理系统也需要相对应的提高.
