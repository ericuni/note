# myisam
每张MyISAM 表被存放在三个文件: frm 文件存放表格定义. 数据文件是MYD (MYData), 索引文件是MYI(MYIndex) 引伸.
因为MyISAM相对简单所以在效率上要优于InnoDB,小型应用使用MyISAM是不错的选择.

MyISAM表是保存成文件的形式,在跨平台的数据转移中使用MyISAM存储会省去不少的麻烦

# innodb
InnoDB 支持事务, 行级锁, 并发性能更好, CPU 及内存缓存页优化使得资源利用率更高.

InnoDB 是聚集索引, MyISAM 是非聚集索引.
聚簇索引要求数据存放在主键索引的叶子节点上, 因此InnoDB 必须要有主键, 通过主键索引效率很高.
但是辅助索引需要两次查询, 先查询到主键, 然后再通过主键查询到数据. 因此, 主键不应该过大, 因为主键太大, 其他索引也都会很大.
而MyISAM 是非聚集索引, 数据文件是分离的, 索引保存的是数据文件的指针. 主键索引和辅助索引是独立的.

InnoDB 把数据和索引存都放在表空间同一个数据文件(可能是多个物理文件)里.

1. MyISAM 则适合用于频繁查询的应用, innodb 的引擎比较适合于插入和更新操作比较多的应用
1. MyISAM 类型不支持事务处理等高级处理, 也不支持外键, 而InnoDB类型支持事务, 回滚, 外键和崩溃恢复能力等高级数据库功能(
  ACID: Atomicity, Consistency, Isolation, Durability)
1. MyISAM 类型的表强调的是性能, 其执行速度比InnoDB类型更快
1. InnoDB 中不保存表的具体行数, 也就是说, 执行`select count(*) from table`时, InnoDB要扫描整个表, 但MyISAM 只要简单的读
  出保存好的行数即可. 但当count 语句包含where 条件时, 两种表的操作是一样的.
1. InnoDB 最小的锁粒度是行锁; MyISAM 最小的锁粒度是表锁, 一个更新语句会锁住整张表, 导致其他查询和更新都会被阻塞, 因此并
  发访问受限. 这也是MySQL 将默认存储引擎从MyISAM 变成InnoDB 的重要原因之一
1. innodb如果是共享表空间, ibdata1文件越来越大

innodb存储引擎可将所有数据存放于`ibdata*`的共享表空间, 也可将每张表存放于独立的.ibd文件的独立表空间.
共享表空间以及独立表空间都是针对数据的存储方式而言的.

- 共享表空间: 某一个数据库的所有的表数据, 索引文件全部放在一个文件中, 默认这个共享表空间的文件路径在data目录下. 默认的文
  件名为:ibdata1 初始化为10M.
- 独立表空间: 每一个表都将会以独立的文件方式来进行存储, 每一个表都有一个.frm表描述文件, 还有一个.ibd文件, 其中这个文件包
  括了单独一个表的数据内容以及索引内容, 默认情况下它的存储位置也是在表的位置之中.

# rocksdb
## rocksdb与innodb的比较
1. innodb空间浪费, B+ tree 分裂导致page 内有较多空闲,尤其是刚分裂完成时, page利用率不高. 对于顺序插入的场景, 块的填充率
  较高.但对于随机场景,每个块的空间利用率就急剧下降了. 反映到整体上就是一个表占用的存储空间远大于实际数据所需空间.
1. innodb现有的压缩效率也不高,压缩以block为单位,也会造成浪费.
1. 写入放大:innodb 更新以页为单位,最坏的情况更新N行会更新N个页.RocksDB append only方式
1. RocksDB对齐开销小:SST file (默认2MB)需要对齐,但远大于4k, `RocksDB_block_size`(默认4k) 不需要对齐,因此对齐浪费空间较少
1. RocksDB索引前缀相同值压缩存储
1. RocksDB占总数据量90%的最底层数据,行内不需要存储系统列seqid
1. MyRocks在MySQL 5.6/5.7就实现了逆序索引,基于逆序的列族实现,显然,逆序索引不能使用默认的default列族.
1. 基于LSM特性,MyRocks还以很低的成本实现了TTL索引,类似于HBase.

