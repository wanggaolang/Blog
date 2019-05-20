/*
 * 借助了深入理解计算机系统的部分代码csapp.h 和csapp.c  ......  o.0
 *     实现http 1.0
 */
#include "csapp.h"
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

void readmulu(char *mulu,int fd);   //读取文件目录,并发给主机
void doit(int fd);   //主机与服务器的交流函数
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs,char *address);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //accept函数的升级版，等待主机连接
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);//服务器回显
	doit(connfd);                                             // //主机与服务器的交流
	Close(connfd);                                            //关闭连接
    }
}

void doit(int fd)  //主机与服务器的交流函数
{
    int is_static;
    struct stat sbuf;
    /*struct stat  
{   
    dev_t       st_dev;      ID of device containing file -文件所在设备的ID  
    ino_t       st_ino;      inode number -inode节点号    
    mode_t      st_mode;     protection -保护模式    
    nlink_t     st_nlink;    number of hard links -链向此文件的连接数(硬连接)    
    uid_t       st_uid;      user ID of owner -user id   
    gid_t       st_gid;      group ID of owner - group id
    dev_t       st_rdev;     device ID (if special file) -设备号，针对设备文件   
    off_t       st_size;     total size, in bytes -文件大小，字节为单位    
    blksize_t   st_blksize;  blocksize for filesystem I/O -系统块的大小    
    blkcnt_t    st_blocks;   number of blocks allocated -文件所占块数   
    time_t      st_atime;    time of last access -最近存取时间    
    time_t      st_mtime;    time of last modification -最近修改时间    
    time_t      st_ctime;    time of last status change -     
};  
stat结构体是文件（夹）信息的结构体，定义如下：以上信息就是可以通过_stat函数获取的所有相关信息，一般情况下，我们关心文件大小和创建时间、访问时间、修改时间。*/
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE],address[MAXLINE],talk[MAXLINE];
    char talk2[MAXLINE],talk3[MAXLINE],fuck[MAXLINE];
    /*typedef struct {
    int rio_fd;                与缓冲区绑定的文件描述符的编号
    int rio_cnt;               缓冲区中还未读取的字节数
    char *rio_bufptr;          当前下一个未读取字符的地址
    char rio_buf[RIO_BUFSIZE]; 内部缓冲区
    } rio_t;*/
    rio_t rio;
    Rio_readinitb(&rio, fd);  //rio_t 结构体初始化,并绑定文件描述符与缓冲区

	//带缓冲输入函数，每次输入一行
 	//从文件rio读出一个文本行(包括结尾的换行符)，将它拷贝到buf，并且用空字符来结束这个文本行
	//最多读MAXLINE-1个字节，余下的一个留给结尾的空字符

    sprintf(talk, "你好，欢迎连接到本服务器\r\n");    //将初始连接的提示写入buf
    sprintf(talk, "%s请输入想获取文件的目录，如/文件夹1/文件夹2/文件夹3：\r\n", talk);
    send(fd, talk, strlen(talk),0); 
if (!Rio_readlineb(&rio, talk2, MAXLINE))  //将rio缓冲区的内容读入talk2
        return;
      sscanf(talk2,"%s",address);	//读取客户所给路径
      printf("客户请求地址为%s\r\n",address);
      readmulu(address,fd);          //读出当前路径的目录发给主机
      sprintf(method,"GET");
     sprintf(version,"HTTP/1.0");
     sprintf(talk, "********************************\r\n");
     sprintf(talk, "%s请输入你想获取的文件,如servers.c\r\n",talk);
     sprintf(talk, "%s********************************\r\n",talk);
     send(fd, talk, strlen(talk),0); 
 if (!Rio_readlineb(&rio, fuck, MAXLINE))  //将rio缓冲区的内容读入fuck
        return;
    sscanf(fuck, "%s", uri);       //将主机说的话解析为请求URI
    printf("客户所请起的文件为：%s\r\n",uri);

    is_static = parse_uri(uri, filename, cgiargs,address);       //利用请求uri得到访问的文件名、CGI参数，并返回是否按照静态网页处理
    if (stat(filename, &sbuf) < 0) {                     //服务器不能找到所请求的文件
	clienterror(fd, filename, "404", "Not found",
		    "servers couldn't find this file");
	return;
    }                                                    

    if (is_static) {   //提供静态内容         
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { //服务器无权访问所请求的文件
	    clienterror(fd, filename, "403", "Forbidden",
			"servers couldn't read the file");
	    return;
	}
	//提供静态内容，即复制合法的路径（filename）里的内容发给主机
	serve_static(fd, filename, sbuf.st_size);        
    }
    else { 	//提供动态内容
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { //检验权限
	    clienterror(fd, filename, "403", "Forbidden",
			"servers couldn't run the CGI program");
	    return;
	}
	//提供动态内容，即复制合法的路径（filename）里的内容发给主机
	serve_dynamic(fd, filename, cgiargs);            
    }
}
//在服务器显示HTTP请求头信息
//为了保持循环，出现重复代码
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];
    Rio_readlineb(rp, buf, MAXLINE);//读取一行数据，放入buf
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {          //回车和换行符表终止输入
	//带缓冲输入函数，每次输入一行
 	//从文件rp读出一个文本行(包括结尾的换行符)，将它拷贝到buf，并且用空字符来结束这个文本行
	//最多读MAXLINE-1个字节，余下的一个留给结尾的空字符
	Rio_readlineb(rp, buf, MAXLINE);
	printf("%s", buf);
    }
    return;
}
//将uri解析为文件名和cgi参数
//若是动态内容，返回0，静态则返回1
int parse_uri(char *uri, char *filename, char *cgiargs,char *address) 
{   char bb[MAXLINE];
    sprintf(bb,"/");
    char *ptr;
    strcat(bb,uri);
    uri=bb;           //在rui前加'/'
    if (!strstr(uri, "cgi-bin")) {  /* 静态内容 */      //将右边参数与左边进行匹配，返回第一次匹配的地址
	strcpy(cgiargs, "");                             //strcpy函数作用为将右面赋值给左面
	strcpy(filename, address);                           //   ./表示当前文件夹
	strcat(filename, uri);                           //将右边内容连接（不覆盖）进左边的内容
	if (uri[strlen(uri)-1] == '/')    //若uri为"/"，则在其后添加根文件，此处为home.html
	    strcat(filename, "home.html");              
	return 1;
    }
    else {  /* 动态内容 */                    //抽取cgi参数   
	ptr = index(uri, '?');               //找出左边字符串中第一次出现右边参数的地址并返回
	if (ptr) {
	    strcpy(cgiargs, ptr+1);	     
	    *ptr = '\0';
	}
	else 
	    strcpy(cgiargs, "");                         
	strcpy(filename, "address");                           
	strcat(filename, uri);                           
	return 0;
    }
}


//提供静态内容，即复制合法的路径里的内容发给主机
void serve_static(int fd, char *filename, int filesize) 
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
    get_filetype(filename, filetype);       //获取文件类型
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //将响应头写进buf中
    sprintf(buf, "%sServer: servers  Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));       //把buf(响应头)写入fd指定的连接套接字描述符
    printf("Response headers:\n");
    printf("%s", buf);

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);    //以只读方式打开请求的文件
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);//将该文件直接读取到虚拟地址空间中的任意位置，起始于srcp，然后关闭文件
    Close(srcfd);                           //关闭文件
    Rio_writen(fd, srcp, filesize);         //把内存中的文件写入fd指定的连接套接字描述符
    Munmap(srcp, filesize);                 //删除刚才在虚拟地址空间申请的内存
}

void get_filetype(char *filename, char *filetype) //获取文件类型，结果给filetype
{
    if (strstr(filename, ".html"))    //将右边参数与左边进行匹配，返回第一次匹配的地址
	strcpy(filetype, "text/html");  //strcpy函数作用为将右面赋值给左面
    else if (strstr(filename, ".gif"))
	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
	strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpeg");
    else
	strcpy(filetype, "text/plain");
}  

void serve_dynamic(int fd, char *filename, char *cgiargs) //动态
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    sprintf(buf, "HTTP/1.0 200 OK\r\n"); //将响应写入buf中
    Rio_writen(fd, buf, strlen(buf));    //将响应写入fd指定套接字描述符
    sprintf(buf, "Server: servers Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
  
    if (Fork() == 0) {      //派生一个新的子进程
	setenv("QUERY_STRING", cgiargs, 1); 
	Dup2(fd, STDOUT_FILENO);         //子进程重定向它的标准输出到已连接文件描述符
	Execve(filename, emptylist, environ); //运行cgi程序
    }
    Wait(NULL); 
}
  
//向主机回显错误信息
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /*将实体体写入body中 */
    sprintf(body, "<html><title>servers Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The servers Web server</em>\r\n", body);
    /*将响应头写入buf中并发给主机*/
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));//将实体体发给主机
}
void readmulu(char *mulu,int fd){
    DIR    *dir;
    struct    dirent    *ptr;
    char message[MAXLINE];
    int i=1;
    dir = opendir(mulu);
    sprintf(message,"当前目录如下\r\n");
    send(fd, message, strlen(message),0);
    while((ptr = readdir(dir)) != NULL){
        sprintf(message,"第%d个文件: %s\r\n",i, ptr->d_name);
	send(fd, message, strlen(message),0);
	i++;
	}
    closedir(dir);
}
/* $end clienterror */
