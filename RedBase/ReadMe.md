这份源码来源自斯坦福大学的Redbase，我做了一定的改进，也基本上通过了一系列的测试。

这份源码里，我删除掉了原来大批量的测试代码，以及gtest的依赖，但是放心，代码是通过了测试才删掉了测试代码的。

改进后的代码和源代码风格相近，但是不完全一致，因为我不太喜欢原来的编码风格，整个代码不太优美，我感觉很难做到优美，就这样吧。我自己写了parser，没用yacc生成，改了一些函数，希望让它们更加通俗易懂，但是效果貌似也不是太好。

所以，如果你想看一下我写的代码的话，还是要对照着Redbase课程提供的说明来看的。

代码不完整，几乎没有做错误处理工作，没做update，insert等一些命令，但是查询命令完成了，为什么会这样呢？因为我懒。查询命令基本是最难啃的一部分了，其余的，肯花时间的话，没有太大难度。

# 如何运行？
首先软件的运行平台是linux，进入该文件夹，执行
```shell
make
```
可以完成代码的编译工作，当前目录下会生成三个程序，分别是**dbcreate**, **RedBase**, **dbdestroy**。

`dbcreate`用于构建数据库，使用方法为:
```bash
./dbcreate dbname  # dbname为数据库名称
```
`dbdestroy`用于删除数据库，使用方法为：
```bash
./dbdestroy dbname # dbname为数据库名称
```
`RedBase`为数据库程序，使用方法为：
```bash
./Redbase dbname
```

这里给一份测试代码：
```bash
/* Test semantic checking.  */

create table soaps(soapid  i, sname  c28, network  c4, rating  f);
create table stars(starid  i, stname  c20, plays  c12, soapid  i);

/* 加载数据 */
load soaps("../data/soaps.data");

/* print out contents of soaps */
print soaps;

/* build some indices on stars */
create index stars(starid);
create index stars(stname);


load stars("../data/stars.data");

print stars;
select * from stars;

select * from stars, soaps;

select soaps.network, rating from stars, soaps;


select soaps.network, rating from stars, soaps where soaps.soapid = stars.soapid;

print stars;

exit;

```
你可以在我代码的基础之上继续改进代码，下面是运行的一些截图：
![](http://img.blog.csdn.net/20171208171016533)
![](http://img.blog.csdn.net/20171206201018250)
