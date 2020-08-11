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

int set_localtime(char *buf)        //更改系统时间
{
    long int time_sec;
    struct tm local; 
    cJSON *obj=NULL;
    cJSON *tmp=NULL,*tmp1=NULL,*tmp2=NULL,*tmp3=NULL,*tmp4=NULL,*tmp5=NULL;
    
    obj=cJSON_Parse(buf);
    if(!obj){
        perror("cJSON_Parse error");
        return -1;
    }

    tmp=cJSON_GetObjectItem(obj,"year");
    tmp1=cJSON_GetObjectItem(obj,"mon");
    tmp2=cJSON_GetObjectItem(obj,"day");
    tmp3=cJSON_GetObjectItem(obj,"hour");
    tmp4=cJSON_GetObjectItem(obj,"min");
    tmp5=cJSON_GetObjectItem(obj,"sec");
    if(!(tmp&&tmp1&&tmp2&&tmp3&&tmp4&&tmp5)){
        perror("cJSON_GetObjectItem error");
        return -1;
    }

    local.tm_year=cJSON_GetNumberValue(tmp)-1900;
    local.tm_mon=cJSON_GetNumberValue(tmp1)-1;
    local.tm_mday=cJSON_GetNumberValue(tmp2);
    local.tm_hour=cJSON_GetNumberValue(tmp3);
    local.tm_min=cJSON_GetNumberValue(tmp4);
    local.tm_sec=cJSON_GetNumberValue(tmp5);


    time_sec=mktime(&local);
    if(time_sec<=0){
        perror("mktime error");
        return -1;
    }

    if(stime(&time_sec)<0){
        perror("stime error");
        return -1;
    }
    cJSON_Delete(obj);
    return 0;
}

int ipcjson_read(char *filename,struct sockaddr_in *myaddr)  //从json文本中读取配置
{
    FILE    *fp=NULL;
    char    line[1024]={0},str[20];
    cJSON   *obj,*addr,*family,*port;
    char    *out_obj,*out_addr;
    short   out_family;
    int     out_port;
 
    if((fp=fopen(filename,"r"))==NULL){
        perror("fopen error");
        return -1;
    }
    fgets(line,1024,fp);
    printf("%s\n",line);

    obj=cJSON_Parse(line);
    addr=cJSON_GetObjectItem(obj,"sin_addr");
    family=cJSON_GetObjectItem(obj,"sin_family");
    port=cJSON_GetObjectItem(obj,"sin_port"); 

    out_addr=cJSON_Print(addr);
    out_family=cJSON_GetNumberValue(family);
    out_port=cJSON_GetNumberValue(port);  

    strncpy(str,out_addr+1,strlen(out_addr)-2);
    str[strlen(out_addr)-2]='\0';
    //printf("%s\n%d\n%d\n",str,out_family,out_port);

    inet_pton(AF_INET,str,&myaddr->sin_addr.s_addr);
    myaddr->sin_port=htons(out_port);
    myaddr->sin_family=out_family;
    memset(myaddr->sin_zero,0,8);

    cJSON_Delete(obj);  
    free(out_addr);
    fclose(fp);
    return 0;
}

int set_noblock(int sock_fd)             //将套接字设置为非阻塞
{
    int flag;
    if(flag = fcntl(sock_fd, F_GETFL, 0) < 0)
    {
        perror("fcntl");
        exit(1);
    }
    flag |= O_NONBLOCK;
    fcntl(sock_fd, F_SETFL, flag ); 
    return flag;
}

bool tcp_test(int sock_fd)               //测试tcp是否在连接状态
{
    struct tcp_info info; 
    int len=sizeof(info);  
    getsockopt(sock_fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len); 
    if(info.tcpi_state==TCP_ESTABLISHED){
        return true;
    }
    else{
        return false;
    }

}
