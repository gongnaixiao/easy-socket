#include<netinet/in.h>                         
#include<sys/types.h>                          
#include<sys/socket.h>                         
#include<stdio.h>                             
#include<stdlib.h>                             
#include<string.h>                             

#define HELLO_WORLD_SERVER_PORT       9999 
#define BUFFER_SIZE                   1024
#define FILE_NAME_MAX_SIZE            512

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s ServerIPAddress\n", argv[0]);
		exit(1);
	}

	/* 设置一个socket地址结构client_addr, 代表客户机的internet地址和端口*/
	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET; /* internet协议族*/
	client_addr.sin_addr.s_addr = htons(INADDR_ANY); /* INADDR_ANY表示自动获取本机地址 */
	client_addr.sin_port = htons(0); /* auto allocated, 让系统自动分配一个空闲端口*/

	/* 创建用于internet的流协议(TCP)类型socket，用client_socket代表客户端socket */
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0)
	{
		printf("Create Socket Failed!\n");
		exit(1);
	}

	/* 把客户端的socket和客户端的socket地址结构绑定*/
	if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))
	{
		printf("Client Bind Port Failed!\n");
		exit(1);
	}

	/* 设置一个socket地址结构server_addr,代表服务器的internet地址和端口 */
	struct sockaddr_in  server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

	/* 服务器的IP地址来自程序的参数 */
	if (inet_aton(argv[1], &server_addr.sin_addr) == 0)
	{
		printf("Server IP Address Error!\n");
		exit(1);
	}

	server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
	socklen_t server_addr_length = sizeof(server_addr);

	/* 向服务器发起连接请求，连接成功后client_socket代表客户端和服务器端的一个socket连接 */
	if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		printf("Can Not Connect To %s!\n", argv[1]);
		exit(1);
	}

	char file_name[FILE_NAME_MAX_SIZE + 1];
	char comm_name[4];
	bzero(file_name, sizeof(file_name));

	printf("Please Input Command:put or get:\t");
	while (fgets(comm_name, 4, stdin))
	{
		if (!strcmp(comm_name, "put") || !strcmp(comm_name, "get"))
			break;
		printf("Wrong Input, Please Input Command: put or get:\t");
	}
	/*
	 * For debug
	 * printf("%s", comm_name);
	 */
	char buffer[BUFFER_SIZE];
   	bzero(buffer, sizeof(buffer));

	if (!strcmp(comm_name, "put"))
	{
		printf("\n");
		printf("Please Input File Name On Client:\t");
		scanf("%s", file_name);

		FILE *fp = fopen(file_name, "r");
		if (fp == NULL)
		{
			printf("File:\t%s Not Found!\n", file_name);
		}

		int file_block_length = 0;
		strcpy(buffer, "put");
		if ((file_block_length = fread(buffer+3, sizeof(char), BUFFER_SIZE-3, fp)) <= 0)
		{
			printf("read error, file_block_length = %d\n", file_block_length);
			exit(-1);
		}
		if (send(client_socket, buffer, (size_t)(file_block_length+3), 0) < 0)
		{
			printf("Send File:\t%s Failed!\n", file_name);
			/*for debug*/
			printf("xg here\n");
			exit(-1);
		}
		bzero(buffer, sizeof(buffer));

		while((file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
		{
			/*for debug*/
			/*printf("xg\n");*/
			printf("file_block_length = %d\n", file_block_length);

			if (send(client_socket, buffer, file_block_length, 0) < 0)
			{
				printf("Send File:\t%s Failed!\n", file_name);
				exit(-1);
			}

			bzero(buffer, sizeof(buffer));
		}
		printf("Send File:\t %s Finished!\n", file_name);
		fclose(fp);
	}
	else if(!strcmp(comm_name, "get"))
	{
		printf("\n");
		printf("Please Input File Name On Server:\t");
		scanf("%s", file_name);
		
		strcpy(buffer, "get");
		strncpy(buffer+3, file_name, strlen(file_name) > BUFFER_SIZE ? BUFFER_SIZE : strlen(file_name));
		/* 向服务器发送buffer中的数据，此时buffer中存放的是客户端需要接收的文件的名字 */
		send(client_socket, buffer, BUFFER_SIZE, 0);

		/* 
		 * FILE *fp = fopen(file_name, "w");
		 */
		/*
		 * set receive filename rec.cli
		 */
		FILE *fp = fopen("rec.cli", "w");
		if (fp == NULL)
		{
			printf("File:\t%s Can Not Open To Write!\n", file_name);
			exit(1);
		}

		/* 从服务器端接收数据到buffer中 */
		bzero(buffer, sizeof(buffer));
		int length = 0;
		while(length = recv(client_socket, buffer, BUFFER_SIZE, 0))
		{
			if (length < 0)
			{
				printf("Recieve Data From Server %s Failed!\n", argv[1]);
				break;
			}

			int write_length = fwrite(buffer, sizeof(char), length, fp);
			if (write_length < length)
			{
				printf("File:\t%s Write Failed!\n", file_name);
				break;
			}
			bzero(buffer, BUFFER_SIZE);
		}

		printf("Recieve File:\t %s From Server[%s] Finished!\n", file_name, argv[1]);

		fclose(fp);
	}

	/* 传输完毕，关闭socket */
	close(client_socket);
	return 0;

}
