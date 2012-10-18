#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>

#define HELLO_WORLD_SERVER_PORT    9999 
#define LENGTH_OF_LISTEN_QUEUE     20
#define BUFFER_SIZE                1024
#define FILE_NAME_MAX_SIZE         512

int main(int argc, char **argv)
{
	/*set socket's address information
	  设置一个socket地址结构server_addr,代表服务器internet的地址和端口
	 */
	struct sockaddr_in   server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

	/* create a stream socket
	   创建用于internet的流协议(TCP)socket，用server_socket代表服务器向客户端提供服务的接口
	 */
	int server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		printf("Create Socket Failed!\n");
		exit(1);
	}

	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		printf("Server Bind Port: %d Failed!\n", HELLO_WORLD_SERVER_PORT);
		exit(1);
	}

	if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
	{
		printf("Server Listen Failed!\n");
		exit(1);
	}

	while(1)
	{
		struct sockaddr_in client_addr;
		socklen_t          length = sizeof(client_addr);

		int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
		if (new_server_socket < 0)
		{
			printf("Server Accept Failed!\n");
			break;
		}

		char buffer[BUFFER_SIZE];
		bzero(buffer, sizeof(buffer));
		length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
		if (length < 0)
		{
			printf("Server Recieve Data Failed!\n");
			break;
		}

		char comm_name[4];
		int i = 0;
		for (i = 0; i < 3; ++i)
		{
			comm_name[i] = buffer[i];
		}
		comm_name[3] = '\0';

		if (!strcmp(comm_name, "get"))
		{
			char file_name[FILE_NAME_MAX_SIZE + 1];
			bzero(file_name, sizeof(file_name));
			strncpy(file_name, buffer+3,
					strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));

			FILE *fp = fopen(file_name, "r");
			if (fp == NULL)
			{
				printf("File:\t%s Not Found!\n", file_name);
			}
			else
			{
				bzero(buffer, BUFFER_SIZE);
				int file_block_length = 0;
				while( (file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
				{
					printf("file_block_length = %d\n", file_block_length);

					if (send(new_server_socket, buffer, file_block_length, 0) < 0)
					{
						printf("Send File:\t%s Failed!\n", file_name);
						break;
					}

					bzero(buffer, sizeof(buffer));
				}
				fclose(fp);
				printf("File:\t%s Transfer Finished!\n", file_name);
			}

			close(new_server_socket);
		}

		if (!strcmp(comm_name, "put"))
		{
			char file_name[FILE_NAME_MAX_SIZE + 1];
			bzero(file_name, sizeof(file_name));
			/*set put filename*/
			strcpy(file_name, "rec.serv");
			FILE *fp = fopen(file_name, "w");
			if (fp == NULL)
			{
				printf("open file error\n");
			}
			else
			{
				int write_length = fwrite(buffer+3, sizeof(char), length-3, fp);
				int flag = 0;
				if (write_length < length-3)
				{
					printf("File:\t%s Write Failed!\n", file_name);                                                                                                  
					flag = 1;
				}

				bzero(buffer, BUFFER_SIZE);                                                                                                                          
				while((length = recv(new_server_socket, buffer, BUFFER_SIZE, 0)) && (flag == 0))
				/*while(length = recv(new_server_socket, buffer, BUFFER_SIZE, 0))*/
				{                                                                                                                                                        
					if (length < 0)                                                                                                                                      
					{                                                                                                                                                    
						printf("Recieve Data From Server %s Failed!\n", argv[1]);                                                                                        
						break;                                                                                                                                           
					}                                                                                                                                                    

					write_length = fwrite(buffer, sizeof(char), length, fp);                                                                                         
					if (write_length < length)                                                                                                                           
					{                                                                                                                                                    
						printf("File:\t%s Write Failed!\n", file_name);                                                                                                  
						break;                                                                                                                                           
					}                                                                                                                                                    
					bzero(buffer, BUFFER_SIZE);                                                                                                                          
				} 
				fclose(fp);
				printf("File:\t%s Transfer Finished!\n", file_name);
			}
			close(new_server_socket);
		}
	}

	close(server_socket);

	return 0;
}

