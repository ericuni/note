- [intro](#intro)
  - [leader election](#leader-election)
  - [log replication](#log-replication)
  - [Safety](#safety)
  - [日志压缩](#日志压缩)
  - [成员变更](#成员变更)

# intro
[Raft算法详解](https://zhuanlan.zhihu.com/p/32052223)

不同于Paxos算法直接从分布式一致性问题出发推导出来,Raft算法则是从多副本状态机的角度提出,用于管理多副本状态机的日志复制.

Raft实现了和Paxos相同的功能,它将一致性分解为多个子问题: Leader选举(Leader election),日志同步(Log replication),安全性(Safety),日志压缩(Log compaction),成员变更(Membership change)等.

同时,Raft算法使用了更强的假设来减少了需要考虑的状态,使之变的易于理解和实现.

Raft将系统中的角色分为领导者(Leader),跟从者(Follower)和候选人(Candidate):

- Leader:接受客户端请求,并向Follower同步请求日志,当日志同步到大多数节点上后告诉Follower提交日志.
- Follower:接受并持久化Leader同步的日志,在Leader告诉日志可以提交之后,提交日志.
- Candidate:Leader选举过程中的临时角色.

Raft算法角色状态转换如下:

<img src="./pics/raft/role.jpg" alt="role transition" width="60%"/>

Follower只响应其他服务器的请求.如果Follower超时没有收到Leader的消息,它会成为一个Candidate并且开始一次Leader选举.
收到大多数服务器投票的Candidate会成为新的Leader.Leader在宕机之前会一直保持Leader的状态.

<img src="./pics/raft/term.jpg" alt="term" width="60%"/>
Raft算法将时间分为一个个的任期(term),每一个term的开始都是Leader选举.在成功选举Leader之后,Leader会在整个term内管理整个集群.如果Leader选举失败,该term就会因为没有Leader而结束.

## leader election
Raft 使用心跳(heartbeat)触发Leader选举.当服务器启动时,初始化为Follower.
如果Follower在选举超时时间内没有收到Leader的heartbeat,就会等待一段随机的时间后发起一次Leader选举.

Follower将其当前term加一然后转换为Candidate.它首先给自己投票并且给集群中的其他服务器发送 RequestVote RPC.
结果有以下三种情况:

1. 赢得了多数的选票,成功选举为Leader,
1. 收到了Leader的消息,表示有其它服务器已经抢先当选了Leader,
1. 没有服务器赢得多数的选票,Leader选举失败,等待选举时间超时后发起下一次选举.

选举出Leader后,Leader通过定期向所有Followers发送心跳信息维持其统治.若Follower一段时间未收到Leader的心跳则认为Leader可能已经挂了,再次发起Leader选举过程.

Raft保证选举出的Leader上一定具有最新的已提交的日志,这一点将Safety 那一节中说明.

## log replication
共识算法的实现一般是基于复制状态机(Replicated state machines),何为复制状态机:
If two identical, deterministic processes begin in the same state and get the same inputs in the same order, they will produce the same output and end in the same state.

简单来说: 相同的初识状态 + 相同的输入 = 相同的结束状态.
引文中有一个很重要的词deterministic,就是说不同节点要以相同且确定性的函数来处理输入,而不要引入一下不确定的值,比如本地时间等.
如何保证所有节点 get the same inputs in the same order, 使用replicated log是一个很不错的idea, log具有持久化, 保序的特点, 是大多数分布式系统的基石.

因此,可以这么说,在raft中,leader将客户端请求(command)封装到一个个log entry,将这些log entries复制(replicate)到所有follower节点,然后大家按相同顺序应用(apply)log entry中的command,则状态肯定是一致的.

<img src="./pics/raft/replicated_state_machine.png" alt="replicated state machine" width="60%"/>

Leader选出后,就开始接收客户端的请求.
Leader把请求作为日志条目(Log entries)加入到它的日志中,然后并行的向其他服务器发起 AppendEntries RPC 复制日志条目.
当这条日志被复制到大多数服务器上,Leader将这条日志应用到它的状态机并向客户端返回执行结果.

<img src="./pics/raft/log_replication.jpg" alt="log replication" width="60%"/>

某些Followers可能没有成功的复制日志,Leader会无限的重试 AppendEntries RPC直到所有的Followers最终存储了所有的日志条目.

Raft日志同步保证如下两点:

1. 如果不同日志中的两个条目有着相同的索引和任期号,则它们所存储的命令是相同的.
1. 如果不同日志中的两个条目有着相同的索引和任期号,则它们之前的所有条目都是完全一样的.

一个Follower可能会丢失掉Leader上的一些条目,也有可能包含一些Leader没有的条目,也有可能两者都会发生.丢失的或者多出来的条目可能会持续多个任期.

Leader通过强制Followers复制它的日志来处理日志的不一致,Followers上的不一致的日志会被Leader的日志覆盖.
Leader为了使Followers的日志同自己的一致,Leader需要从后往前找到Followers同它的日志一致的地方,然后覆盖Followers在该位置之后的条目.

举例说明这个过程,如图所示.

- leader 要把 index 为10的日志复制给 a,则会匹配 index 为9处的 term,即 prevLogIndex 为9,prevLogTerm 为6,此时可以匹配成功,则复制AppendEntry RPC携带的 log
- leader 要把 index 为8的日志复制给e,则会匹配 index 为7处的 term,即 prevLogIndex 为7,prevLogTerm 为5.由于index 为7处的 term 为4,匹配失败,则 leader 会向前搜索并进行匹配,
  直至 index 为5处的 log 匹配成功,则发送6之后所有的 log 给 e
<img src="./pics/raft/log_replication_example.png" alt="log replication example" width="60%"/>

## Safety
State Machine Safety: if a server has applied a log entry at a given index to its state machine, no other server will ever apply a different log entry for the same index.

Raft增加了如下两条限制以保证安全性:

- 拥有最新的已提交的log entry的Follower才有资格成为Leader.
  这个保证是在RequestVote RPC中做的,Candidate在发送RequestVote RPC时,要带上自己的最后一条日志的term和log index,其他节点收到消息时,如果发现自己的日志比请求中携带的更新,则拒绝投票.
  日志比较的原则是,如果本地的最后一条log entry的term更大,则term大的更新,如果term一样大,则log index更大的更新.
- Leader只能推进commit index来提交当前term的已经复制到大多数服务器上的日志,旧term日志的提交要等到提交当前term的日志来间接提交(log index 小于 commit index的日志被间接提交).

之所以要这样,是因为可能会出现已提交的日志又被覆盖的情况:

<img src="./pics/raft/log_rewrite.jpg" alt="log rewrite" width="60%"/>

1. 在阶段a,term为2,S1是Leader,且S1写入日志(term, index)为(2, 2),并且日志被同步写入了S2,
1. 在阶段b,S1离线,触发一次新的选主,此时S5被选为新的Leader,此时系统term为3,且写入了日志(term, index)为(3, 2);
1. S5尚未将日志推送到Followers就离线了,进而触发了一次新的选主,而之前离线的S1经过重新上线后被选中变成Leader,此时系统term为4,此时S1会将自己的日志同步到Followers,
  按照上图就是将日志(2, 2)同步到了S3,而此时由于该日志已经被同步到了多数节点(S1, S2, S3),因此,此时日志(2,2)可以被提交了.,
1. 在阶段d,S1又下线了,触发一次选主,而S5有可能被选为新的Leader(这是因为S5可以满足作为主的一切条件:1. term = 5 > 4,2. 最新的日志为(3,2),比大多数节点(如S2/S3/S4的日志都新),
  然后S5会将自己的日志更新到Followers,于是S2,S3中已经被提交的日志(2,2)被截断了.

增加上述限制后,即使日志(2,2)已经被大多数节点(S1,S2,S3)确认了,但是它不能被提交,因为它是来自之前term(2)的日志,直到S1在当前term(4)产生的日志(4, 4)被大多数Followers确认,
S1方可提交日志(4,4)这条日志,当然,根据Raft定义,(4,4)之前的所有日志也会被提交.
此时即使S1再下线,重新选主时S5不可能成为Leader,因为它没有包含大多数节点已经拥有的日志(4,4).

究其根本,是因为term4时的leader s1在(C)时刻提交了之前term2任期的日志.为了杜绝这种情况的发生:

Raft never commits log entries from previous terms by counting replicas.
Only log entries from the leader's current term are committed by counting replicas;
once an entry from the current term has been committed in this way, then all prior entries are committed indirectly because of the Log Matching Property.

也就是说,某个leader选举成功之后,不会直接提交前任leader时期的日志,而是通过提交当前任期的日志的时候"顺手"把之前的日志也提交了,具体怎么实现,在log matching部分有详细介绍.
那么问题来了,如果leader被选举后没有收到客户端的请求呢,论文中有提到,在任期开始的时候发立即尝试复制,提交一条空的log

Raft handles this by having each leader commit a blank no-op entry into the log at the start of its term.

因此,在上图中,不会出现(C)时刻的情况,即term4任期的leader s1不会复制term2的日志到s3.而是如同(e)描述的情况,通过复制-提交 term4的日志顺便提交term2的日志.
如果term4的日志提交成功,那么term2的日志也一定提交成功,此时即使s1crash,s5也不会重新当选.

## 日志压缩
在实际的系统中,不能让日志无限增长,否则系统重启时需要花很长的时间进行回放,从而影响可用性.
Raft采用对整个系统进行snapshot来解决,snapshot之前的日志都可以丢弃.
每个副本独立的对自己的系统状态进行snapshot,并且只能对已经提交的日志记录进行snapshot.

Snapshot中包含以下内容:

- 日志元数据.最后一条已提交的 log entry的 log index和term.这两个值在snapshot之后的第一条log entry的AppendEntries RPC的完整性检查的时候会被用上.
- 系统当前状态.

当Leader要发给某个日志落后太多的Follower的log entry被丢弃,Leader会将snapshot发给Follower.或者当新加进一台机器时,也会发送snapshot给它.
发送snapshot使用InstalledSnapshot RPC.

## 成员变更
成员变更是在集群运行过程中副本发生变化,如增加/减少副本数,节点替换等.

为了解决这一问题,Raft提出了两阶段的成员变更方法.
集群先从旧成员配置Cold切换到一个过渡成员配置,称为共同一致(joint consensus),共同一致是旧成员配置Cold和新成员配置Cnew的组合Cold U Cnew,一旦共同一致Cold U Cnew被提交,系统再切换到新成员配置Cnew.

<img src="./pics/raft/node_change.jpg" alt="raft 两阶段成员变更" width="60%"/>

Raft两阶段成员变更过程如下:

1. Leader收到成员变更请求从Cold切成Cnew,
1. Leader在本地生成一个新的log entry,其内容是Cold ∪ Cnew,代表当前时刻新旧成员配置共存,写入本地日志,同时将该log entry复制至Cold∪Cnew中的所有副本.
  在此之后新的日志同步需要保证得到Cold和Cnew两个多数派的确认,
1. Follower收到Cold∪Cnew的log entry后更新本地日志,并且此时就以该配置作为自己的成员配置,
1. 如果Cold和Cnew中的两个多数派确认了Cold U Cnew这条日志,Leader就提交这条log entry,
1. 接下来Leader生成一条新的log entry,其内容是新成员配置Cnew,同样将该log entry写入本地日志,同时复制到Follower上,
1. Follower收到新成员配置Cnew后,将其写入日志,并且从此刻起,就以该配置作为自己的成员配置,并且如果发现自己不在Cnew这个成员配置中会自动退出,
1. Leader收到Cnew的多数派确认后,表示成员变更成功,后续的日志只要得到Cnew多数派确认即可.Leader给客户端回复成员变更执行成功.

异常分析:

- 如果Leader的Cold U Cnew尚未推送到Follower,Leader就挂了,此后选出的新Leader并不包含这条日志,此时新Leader依然使用Cold作为自己的成员配置.
- 如果Leader的Cold U Cnew推送到大部分的Follower后就挂了,此后选出的新Leader可能是Cold也可能是Cnew中的某个Follower.
- 如果Leader在推送Cnew配置的过程中挂了,那么同样,新选出来的Leader可能是Cold也可能是Cnew中的某一个,此后客户端继续执行一次改变配置的命令即可.
- 如果大多数的Follower确认了Cnew这个消息后,那么接下来即使Leader挂了,新选出来的Leader肯定位于Cnew中.

两阶段成员变更比较通用且容易理解,但是实现比较复杂,同时两阶段的变更协议也会在一定程度上影响变更过程中的服务可用性,因此我们期望增强成员变更的限制,以简化操作流程.

两阶段成员变更,之所以分为两个阶段,是因为对Cold与Cnew的关系没有做任何假设,为了避免Cold和Cnew各自形成不相交的多数派选出两个Leader,才引入了两阶段方案.

如果增强成员变更的限制,假设Cold与Cnew任意的多数派交集不为空,这两个成员配置就无法各自形成多数派,那么成员变更方案就可能简化为一阶段.

那么如何限制Cold与Cnew,使之任意的多数派交集不为空呢?方法就是每次成员变更只允许增加或删除一个成员.

可从数学上严格证明,只要每次只允许增加或删除一个成员,Cold与Cnew不可能形成两个不相交的多数派.

一阶段成员变更:

1. 成员变更限制每次只能增加或删除一个成员(如果要变更多个成员,连续变更多次).
1. 成员变更由Leader发起,Cnew得到多数派确认后,返回客户端成员变更成功.
1. 一次成员变更成功前不允许开始下一次成员变更,因此新任Leader在开始提供服务前要将自己本地保存的最新成员配置重新投票形成多数派确认.
1. Leader只要开始同步新成员配置,即可开始使用新的成员配置进行日志同步.
