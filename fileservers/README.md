零.实现功能
	http/1.0
	静态文件服务器,输入参数为路径+文件名
	目录功能
	线程

一.运行方法
	./fileservers 端口
	在另一个终端：nc 127.0.0.1 端口
	输入想打开文件的路径，如/home
	输入想打开文件，如home.html


二.若想先编译在运行，执行gcc csapp.h csapp.c  servers.c -lpthread -o 程序名

三.参考资料
	c语言程序设计
	深入理解计算机系统
	计算机网络自顶向下
	Tinyhttpd————不到 500 行的超轻量型 Http Server

四.感想
收获挺多的，没学过的函数真的多。一半时间写代码，一半时间找bug。如果还有bug请大佬指正[:)]

