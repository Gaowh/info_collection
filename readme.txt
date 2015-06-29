
#####源代码目录：

1、 decode 目录
	一些基本的解码函数，路由器上没有可以直接解码的API,所以这里是自己写的

2、hash-table目录
	下是一些hash表的操作，由于在搜索时可能产生很多冗余（一次搜索，多次相同的数据），哈希表用于解决这个问题，尽量一次搜索得到一次数据。该目录下还有一个线程 ，hash_func用于管理哈希表，和程序的配置重读

3、include 目录
	一些头文件

4、lib 目录
	两个动态库，路由器上没有，安装到路由器上即可（自动安装，不用管）

5、log 目录
	一个简单的日志记录程序，记录程序在运行时的一些错误，或者关键信息。

6、message 目录
	当搜集到的信息到达一定的量时，调用该文件的API向服务器发送打包好的数据包（现在服务器地址是直接写死在程序里（include/task.h文件内），之后会通过config配置文件来管理，支持多个多个服务器地址（备用））

7、pthread-func 目录
	该目录下是具体的提取搜索关键词的文件，主要是一个线程函数和很多针对不同网站的关键词提取函数

8、scripts 目录
	该目录下是两个脚本文件，用于设置和清除iptables，程序内部调用，无需手动调用

9、task 目录
	该目录是一些线程需要调用的API，无关重要



####安装过后的目录

1、安装在路由器后，该进程对应的目录是/bh_manage/info_collection

2、在info_collection目录下， 有一个scripts目录， 存放的是iptables脚本

3、在info_collection目录下，有一个config文件，是用于配置的文件

4、在info_collction目录下，info.log文件是日志文件，用于记录程序运行时的关键信息或者错误
	

#####运行：

1、可运行文件的名字为 info_collection,安装后直接运行即可，不需要任何参数

2、config文件
	目前支持配置的只有iptables选项（之后会加入另外的配置选项），配置收集哪些购物网站的搜索关键词，具体位置在/bh_manage/info_collection/config(路由器上的具体位置),具体格式文件里很清楚。默认所有的手机APP类都选了，电脑网页的没选

	重新配置了config文件时， 在命令行用命令kill -s USR1 pid 让程序重新读取配置文件

4、关闭程序
	用kill pid  ，这样可以在关闭进程的同时清除iptables

5、进程意外退出
	必须手动清理iptables, 命令如下，必须按顺序执行
	 1)
	 iptables -t filter -D FORWARD -i br-lan -p tcp --dport 80 -m state
	 --state ESTABLISHED -j info_chain
	 2)
	 iptables -F info_chain
	 iptables -X info_chain

6、程序的一些日志记录在 /bh_manage/info_collection/info.log 文件中



####注意
1、需要iptables 的string模块的支持，一般编译的时候都选了的， 如果没选，选上就行

2、当同时统计电脑和手机的搜索时，偶尔（几乎不）会出现错误（乱码）情况，这个问题之后再解决


###增加
1、新增了京东的手机网页的搜索（和APP搜索时不同）
2、新增了一号店手机网页搜索的支持
3、测试淘宝天猫客户端和网页都相同
4、测试苏宁客户端和网页相同

5、匹配方式由原来的strstr变为了kmp算法进行的匹配，在速度上应该会有一定的提升

6、同步时间，用到了ntpdate命令，所以在编译时需要选择ntpdate

7、message/message.c该了一些， 可以过滤掉一部分搜索不正确的信息

8、将服务器地址和程序最多保存的消息数作为配置文件读入

###编译选项

1、需要选择ulog， /ulog查找位置
2、需要选择ntpdate
3、需要选择iptables string 模块
