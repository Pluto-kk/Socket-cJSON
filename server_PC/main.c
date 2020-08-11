#pragma comment(lib,"ws2_32.lib")    //�����Ŀ������
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include <string.h>
#include <WS2tcpip.h>
#include "cJSON.h"

#define PORT 8080
#define MAX_QUEUE_WAIT 5

void send_localtime(int cli_fd);  //���ͱ���ʱ�䣨json��ʽ��

int main()
{
	WSADATA wsa;
	/*��ʼ��socket��Դ*/
	if (WSAStartup(MAKEWORD(1, 1), &wsa) != 0) {
		return;   //����ʧ��
	}
	if (LOBYTE(wsa.wVersion) != 1 ||
		HIBYTE(wsa.wVersion) != 1) {
		WSACleanup();
		return;
	}

	SOCKADDR_IN host_addr;
	char  currtime[20] = {0};

	SOCKET listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		perror("Create socket fd error");
		exit(1);
	}

	
	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(PORT);
	host_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(listen_fd, (SOCKADDR*)&host_addr, sizeof(SOCKADDR)) < 0) {
		perror("bind error");
		exit(1);
	}

	unsigned value = 1;
	//���ö˿ڸ���
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&value, sizeof(value)) != 0) {
		perror("setsockopt error");
		exit(1);
	}

	if (listen(listen_fd, MAX_QUEUE_WAIT) < 0) {
		perror("listen error");
		exit(1);
	}

	int cli_fd;
	int clien_size;
	SOCKADDR_IN clien_addr;
	clien_size = sizeof(SOCKADDR_IN);

	while (1) {
		cli_fd = accept(listen_fd, (SOCKADDR*)&clien_addr, &clien_size);
		if (cli_fd < 0) {
			printf("connect fail\n");
		}
		else {
			char str[20];
			if (!inet_ntop(AF_INET, &clien_addr.sin_addr, str, 20)) {
				perror("inet_ntop error");
				exit(1);
			}
			printf("Connected IP=[ %s ] PORT:%d  sockfd=%d\n", str, ntohs(clien_addr.sin_port), cli_fd);
			while (1) {
				char buf[1000];
				int num = recv(cli_fd, buf, sizeof(buf), 0);
				if (num == 0) {
					printf("�����ѶϿ�\n");
					break;
				}
				else if (num < 0) {
					printf("recv error\n");
					exit(1);
				}
				else {
					buf[num] = '\0';
					printf("Recv<<%s\n", buf);
					send_localtime(cli_fd);
				}
			}
		}
	}
	return 0;
}

void send_localtime(int cli_fd)      //���ͱ���ʱ�䣨json��ʽ��
{
	char *str=NULL;
	time_t mytime;
	struct tm* local;
	mytime = time(NULL);	
	local = localtime(&mytime);
	//sprintf(str,"%04d-%02d-%02d %02d:%02d:%02d",local->tm_year+ 1900, local->tm_mon+1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);

	cJSON *obj;
	obj = cJSON_CreateObject();

	cJSON_AddNumberToObject(obj,"year",local->tm_year+1900);
	cJSON_AddNumberToObject(obj, "mon", local->tm_mon+1);
	cJSON_AddNumberToObject(obj, "day", local->tm_mday);
	cJSON_AddNumberToObject(obj, "hour", local->tm_hour);
	cJSON_AddNumberToObject(obj, "min", local->tm_min);
	cJSON_AddNumberToObject(obj, "sec", local->tm_sec);

	str = cJSON_Print(obj);

	send(cli_fd, str,strlen(str),0);
	printf("Send>>%s\n",str);
	cJSON_Delete(obj);
	free(str);
}