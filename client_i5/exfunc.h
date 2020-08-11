#ifndef _EXFUNC_H_
#define _EXFUNC_H_

int ipcjson_read(char *filename,struct sockaddr_in *myaddr);     //从json文档中读取配置
int set_noblock(int sock_fd);                //将套接字设置为非阻塞
bool tcp_test(int sock_fd);                  //测试tcp是否在连接状态
int set_localtime(char *);

#endif