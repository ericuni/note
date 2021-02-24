- [create](#create)
	- [复制数据到新表](#复制数据到新表)
- [insert](#insert)
- [update](#update)
- [alter](#alter)
- [select](#select)
	- [limit](#limit)
	- [group by, where, having](#group-by-where-having)
	- [join](#join)
		- [INNER JOIN (SIMPLE JOIN)](#inner-join-simple-join)
		- [LEFT OUTER JOIN](#left-outer-join)
		- [RIGHT OUTER JOIN](#right-outer-join)
		- [FULL OUTER JOIN](#full-outer-join)

# create
```sql
show create table tb;  // 查看建表语句
```

必须使用utf8mb4 字符集, MySQL 中的UTF-8 并非真正的UTF-8, 而utf8mb4 才是真正的UTF-8.

必须把字段定义为 NOT NULL 并且提供默认值

1. NULL 的列使索引/索引统计/值比较都更加复杂, 对MySQL 来说更难优化.
1. NULL 这种类型MySQL 内部需要进行特殊处理, 增加数据库处理记录的复杂性, 同等条件下, 表中有较多空字段的时候, 数据库的处理
  性能会降低很多.
1. NULL 值需要更多的存储空间, 无论是表还是索引中每行中的NULL 的列都需要额外的空间来标识.

## 复制数据到新表
- 复制表结构(包括index, key 等)
	1. `CREATE TABLE 新表 LIKE 旧表`
- 复制数据
	1. `CREATE TABLE 新表 SELECT * FROM 旧表`
	1. `INSERT INTO 新表 SELECT * FROM 旧表`

# insert
- `INSERT INTO table_name VALUES(value1, value2, value3, ...);`
- `INSERT INTO table_name(column1, column2, column3, ...) VALUES(value1, value2, value3, ...);`

MySql避免重复插入记录(根据主键判重)

1. `insert ignore into xxx`
1. `replace into xxx`

# update
```sql
UPDATE table_name SET column1 = value1, column2 = value2, ...  WHERE some_column = some_value;
```

如果update 的一条记录在数据库不存在(就是说后面的where语句没有找到record), 不会对数据库产生影响, 同时语句的执行也不会报错.

两个表update

- `update a set age = (select age from b where b.name = a.name)`: 用到了子查询, 当数据量大的时候, 效率非常低
- `update a, b set a.age = b.age where a.name = b.name`: 比上面的子查询效率会高很多, 尤其是在建立有合适的索引时.

# alter
```sql
ALTER TABLE table_name ADD column_name datatype
alter table flows add column ip_prot tinyint(4) null default 0;

ALTER TABLE table_name DROP COLUMN column_name

ALTER TABLE table_name MODIFY COLUMN column_name datatype

ALTER TABLE table_name CHANGE old_col_name new_col_name column_definition
ALTER TABLE table_name RENAME TO new_table_name;
```

# select
mysql中提供了一个G标志,放到sql语句后,可以使一行的每个列打印到单独的行.
```sql
select * from t_goods \G
```

## limit
mysql不支持`select top n`的语法,应该用这个替换:
```sql
select * from tablename order by orderfield desc/asc limit offset, counter;
```
offset 指示从哪里开始查询,如果是0则是从头开始, counter 表示查询的个数

为了检索从某一个偏移量到记录集的结束所有的记录行,可以指定counter 为 -1:

## group by, where, having
可以在包含 GROUP BY 子句的查询中使用 WHERE 子句.在完成任何分组之前,将消除不符合 WHERE 子句中的条件的行.例如:
```sql
SELECT ProductModelID, AVG(ListPrice) AS 'Average List Price'
FROM Production.Product
WHERE ListPrice > $1000
GROUP BY ProductModelID
ORDER BY ProductModelID;
```

having子句与where有相似之处但也有区别,都是设定条件的语句.

1. **having只能用在group by之后,对分组后的结果进行筛选**(即使用having的前提条件是分组).
2. where肯定在group by 之前, 即也在having之前
3. where后的条件表达式里不允许使用聚合函数,而having可以

在查询过程中优先级
```sql
where 子句 > 聚合语句(sum, min, max, avg, count) > having子句
```
eg:

1. `select sum(num) as rmb from order where id>10` 只有对id大于10的记录才对进行聚合语句
1. `select reportsto as manager, count(*) as reports from employees group by reportsto having count(*) > 4`
首先将reportsto相同的group起来, 统计各个group里面成员的个数, 然后再筛选出大于4的groups.
如果把上面的having换成where则会出错.

统计分组数据时用到聚合语句, 对分组数据再次判断时要用having. 如果不用这些关系就不存在使用having, 直接使用where就行了.
having就是来弥补where在分组数据判断时的不足. 因为where执行优先级别要快于聚合语句.

当一个查询语句同时出现了where,group by,having,order by的时候,执行顺序和编写顺序是:

1. 执行where xx对全表数据做筛选,返回第1个结果集
2. 针对第1个结果集使用group by分组,返回第2个结果集
3. 针对第2个结果集中的每1组数据执行select xx,有几组就执行几次,返回第3个结果集
4. 针对第3个结集执行having xx进行筛选,返回第4个结果集
5. 针对第4个结果集排序

eg: 完成一个复杂的查询语句,需求如下: 按由高到低的顺序显示个人平均分在70分以上的学生姓名和平均分,
为了尽可能地提高平均分,在计算平均分前不包括分数在60分以下的成绩.

分析

1. 要求显示学生姓名和平均分, 因此确定第1步`select s_name,avg(score) from student`
2. 计算平均分前不包括分数在60分以下的成绩,因此确定第2步 `where score>=60`
3. 显示个人平均分, 相同名字的学生(同一个学生)考了多门科目,因此按姓名分组.确定第3步 `group by s_name`
4. 显示个人平均分在70分以上 因此确定第4步 `having avg(s_score)>=70`
5. 按由高到低的顺序, 因此确定第5步 `order by avg(s_score) desc`

合起来就是:
```sql
select s_name,avg(s_score) from student
where score>=60
group by s_name
having avg(s_score)>=70
order by avg(s_score) desc
```

## join
[sql joins](http://www.techonthenet.com/sql/joins.php)

There are 4 different types of SQL joins:

- SQL INNER JOIN (or sometimes called simple join)
- SQL LEFT OUTER JOIN (or sometimes called LEFT JOIN)
- SQL RIGHT OUTER JOIN (or sometimes called RIGHT JOIN)
- SQL FULL OUTER JOIN (or sometimes called FULL JOIN)

### INNER JOIN (SIMPLE JOIN)
It is the most common type of SQL join.
SQL INNER JOINS return all rows from multiple tables where the join condition is met.

The syntax for the SQL INNER JOIN is:
```sql
SELECT columns
FROM table1
INNER JOIN table2
ON table1.column = table2.column;
```
In this visual diagram, the SQL INNER JOIN returns the shaded area:

<img src="./pics/sql/inner_join.gif" alt="inner join" width="20%"/>

The SQL INNER JOIN would return the records where table1 and table2 intersect.

### LEFT OUTER JOIN
This type of join returns all rows from the LEFT-hand table specified in the ON condition and only those rows from the
other table where the joined fields are equal (join condition is met).

In some databases, the LEFT OUTER JOIN keywords are replaced with LEFT JOIN.

<img src="./pics/sql/left_outer_join.gif" alt="left outer join" width="20%"/>

### RIGHT OUTER JOIN
This type of join returns all rows from the RIGHT-hand table specified in the ON condition and only those rows from the
other table where the joined fields are equal (join condition is met).

<img src="./pics/sql/right_outer_join.gif" alt="right outer join" width="20%"/>

### FULL OUTER JOIN
This type of join returns all rows from the LEFT-hand table and RIGHT-hand table
**with nulls in place where the join condition is not met**.

<img src="./pics/sql/full_outer_join.gif" alt="full outer join" width="20%"/>
