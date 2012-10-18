#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/errno.h>

#define HELLO_WORLD_SERVER_PORT     9999
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define THREAD_MAX    5
void * talk_to_client(void *data)
{
	int new_server_socket = (int)data;
	char buffer[BUFFER_SIZE];

	bzero(buffer, BUFFER_SIZE);
	strcpy(buffer,"Hello,World! 从服务器来！");
	strcat(buffer,"\n"); 
	send(new_server_socket,buffer,BUFFER_SIZE,0);

	bzero(buffer,BUFFER_SIZE);
	/*接收客户端发送来的信息到buffer中*/
	char file_name[BUFFER_SIZE];
	memset(file_name, 0x00, sizeof(file_name));
	int length = 0;
	length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
	if (length <= 0)
	{
		printf("recv erro\n");
		exit(-1);
	}
	int i=0;
	for (i=0; i<10; ++i)
	{
		file_name[i] = buffer[i];
	}
	file_name[10] = '\0';

	/*
	int file_int = atoi(file_name);
	sprintf(file_name, "%d", file_int);
	*/
	FILE *fp = fopen(file_name, "w");
	fwrite(buffer+10, sizeof(char), length-10, fp);
	/*for debug*/
	/*
	printf("[%s]", buffer);
	*/

	while ((length = recv(new_server_socket,buffer,BUFFER_SIZE,0)) > 0)
	{
		/*printf("\nSocket Num: %d \t %s",new_server_socket, buffer);*/

		/*写文件*/
		/*printf("xg here\n");*/
		fwrite(buffer, sizeof(char), length, fp);	
	}
	fclose(fp);

	/*关闭与客户端的连接*/
	close(new_server_socket); 
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	/*设置一个socket地址结构server_addr,代表服务器internet地址, 端口*/
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr)); 
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

	/*创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket*/
	int server_socket = socket(AF_INET,SOCK_STREAM,0);
	if( server_socket < 0)
	{
		printf("Create Socket Failed!");
		exit(1);
	}

	/*把socket和socket地址结构联系起来*/
	if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)) )
	{
		printf("Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT); 
		exit(1);
	}

	/*server_socket用于监听*/
	if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) )
	{
		printf("Server Listen Failed!"); 
		exit(1);
	}

	int i;
	while(1) 
	{

		/*定义客户端的socket地址结构client_addr*/
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);

		/*接受一个到server_socket代表的socket的一个连接
		如果没有连接请求,就等待到有连接请求--这是accept函数的特性
		accept函数返回一个新的socket,这个socket(new_server_socket)用于同连接到的客户的通信
		new_server_socket代表了服务器和客户端之间的一个通信通道
		accept函数把连接到的客户端信息填写到客户端的socket地址结构client_addr中
		*/
		int new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);
		if ( new_server_socket < 0)
		{
			printf("Server Accept Failed!\n");
			break;
		}
		pthread_t child_thread;
		pthread_attr_t child_thread_attr;
		pthread_attr_init(&child_thread_attr);
		pthread_attr_setdetachstate(&child_thread_attr,PTHREAD_CREATE_DETACHED);
		if( pthread_create(&child_thread,&child_thread_attr,talk_to_client, (void *)new_server_socket) < 0 )
		/*if( pthread_create(&child_thread,NULL,talk_to_client, (void *)new_server_socket) < 0 )*/
			printf("pthread_create Failed : %s\n",strerror(errno));
	}

	/*关闭监听用的socket*/
	close(server_socket);

	return 0;

}
