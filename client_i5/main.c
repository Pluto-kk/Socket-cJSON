#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "cJSON.h"
#include "exfunc.h"

#define PORT 8080
#define MAXIMUM_QUEUE_WAIT  10

void * pth1_function(void * arg);

pthread_mutex_t sock_lock;          //互斥锁

int main(int argc,char **argv)
{
    int sock_fd; 
    struct sockaddr_in server_addr;            //服务器网络地址结构体
    sock_fd=socket(AF_INET,SOCK_STREAM,0);     //流式套接字
    if(sock_fd<0){
        perror("Create socket fd error");
        return 0;
    }

    int value = 1;
    //设置端口复用
    if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value))!=0){
        perror("setsockopt error");
        exit(1);
    }

    //从json文本中读取配置
    if(ipcjson_read("ipcjson.txt",&server_addr)<0){
        perror("ipcjson_read error");
        exit(1);
    }

    printf("connect...\n");
    //connect连接
    int connct_num=connect(sock_fd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr));
    if(connct_num<0){
        perror("connect error");
        exit(1);
    }
    printf("connect success%d\n",sock_fd);

    pthread_mutex_init(&sock_lock,NULL);                    //初始化互斥锁
    pthread_t pth1;                             
    pthread_create(&pth1, NULL, pth1_function, &sock_fd);   //创建线程

    while(1)
    {  
        char read_buf[1000];

        //套接字和终端输入设置为非阻塞
        int flag=set_noblock(sock_fd);
        pthread_mutex_lock(&sock_lock);
        //接收数据
        int num=recv(sock_fd,read_buf,sizeof(read_buf),0);
        if(num>0){
            read_buf[num]='\0';
            printf("recv<<%s\n",read_buf);
            if(set_localtime(read_buf)<0){
                perror("set time fail ");
                exit(1);
            }
        } 
        if(!tcp_test(sock_fd)){
            //printf("test\n");
            pthread_cancel(pth1);               //取消线程
            pthread_join(pth1,NULL);            //回收线程资源
            flag &= ~O_NONBLOCK;
            fcntl(sock_fd, F_SETFL, flag);        
            close(sock_fd);
            break;
        }
        pthread_mutex_unlock(&sock_lock);
    }
    printf("conncted break\n");
    return 0;
}

void * pth1_function(void * arg)
{
    int sock_fd = *(int*)arg;
    int num;
    char * str=NULL;

    //创建json格式的心跳消息文本
    cJSON *tmp;
    tmp=cJSON_CreateObject();
    cJSON_AddStringToObject(tmp,"Heart message","Time");
    str=cJSON_Print(tmp);
    cJSON_Delete(tmp);

    while(1){
        pthread_testcancel();       //设置取消点
        sleep(2);

        pthread_mutex_lock(&sock_lock);
        //发送心跳消息
        num=send(sock_fd,str,strlen(str),0);
        if(num>0){
            printf("Send>>%s\n",str);
        }
        pthread_mutex_unlock(&sock_lock);
    }
}



