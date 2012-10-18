#include <netinet/in.h>    
#include <sys/types.h>    
#include <sys/socket.h>   
#include <stdio.h>       
#include <stdlib.h>     
#include <string.h>    
#include <pthread.h>
#include <sys/errno.h>  

#define HELLO_WORLD_SERVER_PORT    9999 
#define BUFFER_SIZE 1024

char * server_IP = NULL;
struct st_pthread_type
{
	int i;
	char file_name[BUFFER_SIZE];
};

void * talk_to_server(void *pst_pthread)
{
	/*设置一个socket地址结构client_addr,代表客户机internet地址, 端口*/
	struct sockaddr_in client_addr;
	bzero(&client_addr,sizeof(client_addr)); 
	client_addr.sin_family = AF_INET;   
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
	client_addr.sin_port = htons(0);    
	/*创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket*/
	int client_socket = socket(AF_INET,SOCK_STREAM,0);
	if( client_socket < 0)
	{
		printf("Create Socket Failed!\n");
		exit(1);
	}
	/*把客户机的socket和客户机的socket地址结构联系起来*/
	if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
	{
		printf("Client Bind Port Failed!\n"); 
		exit(1);
	}

	/*设置一个socket地址结构server_addr,代表服务器的internet地址, 端口*/
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	if(inet_aton(server_IP,&server_addr.sin_addr) == 0) 
	{
		printf("Server IP Address Error!\n");
		exit(1);
	}
	server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
	socklen_t server_addr_length = sizeof(server_addr);
	/*向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接*/
	if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		printf("Can Not Connect To %s!\n",server_IP);
		exit(1);
	}

	char buffer[BUFFER_SIZE];
	bzero(buffer,BUFFER_SIZE);
	/*从服务器接收数据到buffer中*/
	int length = recv(client_socket,buffer,BUFFER_SIZE,0);
	if(length < 0)
	{
		printf("Recieve Data From Server %s Failed!\n", server_IP);
		exit(1);
	}
	struct st_pthread_type *pst_pthread_tmp = (struct st_pthread_type *)pst_pthread;
	printf("From Server %s :\t%s\t Client Thread Num:%d\n",server_IP,buffer, ((struct st_pthread_type *)pst_pthread)->i);
	bzero(buffer,BUFFER_SIZE);
	
	/*读文件*/
	FILE *fp = fopen(pst_pthread_tmp->file_name, "r");
	int read_length;
	sprintf(buffer, "%s00", pst_pthread_tmp->file_name);
	read_length = fread(buffer+10, sizeof(char), BUFFER_SIZE, fp);

	/*for debug*/
	int tmp = send(client_socket, buffer, read_length, 0);
	printf("send = %d", tmp);

	bzero(buffer,BUFFER_SIZE);
	while ((read_length = fread(buffer, sizeof(char), BUFFER_SIZE,fp)) > 0)
	{
		/*向服务器发送buffer中的数据*/
		send(client_socket,buffer,read_length,0);
		bzero(buffer,BUFFER_SIZE);
	}
	/*关闭socket*/
	close(client_socket);
	pthread_exit(NULL);
}

int file_parting(FILE *fp_original, struct st_pthread_type st_pthread[])
{
	char buffer[BUFFER_SIZE];
	char file_name[BUFFER_SIZE];
	FILE *fp_tmp = fp_original;
	FILE *tfp;
	/*
	   while (fread(buffer, sizeof(char), BUFFER_SIZE, fp_tmp) > 0)
	   {
	   fwrite(fp[i]);
	   }
	 */ 		
	/*分割文件个数*/
	int count = 5;
	/*文件大小*/
	int size; 		
	/*每个分割文件有几个buffer*/
	int read_count = 0;

	fseek(fp_tmp, 0, SEEK_END);	
	size = ftell(fp_tmp);
	int each_size = size/5;
	int last_size = size - (each_size * 4);

	/*读取分割文件的最大次数*/
	int read_size = each_size > BUFFER_SIZE ? BUFFER_SIZE : each_size;	
	read_count = last_size / read_size;

	/*for debug*/	
	/*
	printf("%d, %d, %d", size, each_size, read_size);
	*/

	if (each_size % read_size)
	{
		read_count += 1;
	}
	printf("read_count=[%d]\n", read_count);

	fseek(fp_tmp, 0, SEEK_SET);
	int i = 0;
	int read_length = 0;
	int temp = read_count;
	for (i=0; i < count-1; ++i)
	{
		sprintf(file_name, "%d", i);
		strcpy(st_pthread[i].file_name, file_name);
		tfp = fopen(file_name, "w");
		while (temp--)
		{
			/*
			   fscanf(fp, "%s", buffer);
			   fprintf(fp[i],"%s", buffer);
			 */
			read_length = fread(buffer, sizeof(char), read_size, fp_tmp);
			if (read_length > 0)
			{
				fwrite(buffer, sizeof(char), read_length, tfp);
			}
			else
			{
				break;
			}
		}
		temp = read_count;
		fclose(tfp);
		/*for debug*/
		/*
		fseek(tfp, 0, SEEK_END);
		printf("size = %d\n", ftell(tfp));
		*/
	}

	sprintf(file_name, "%d", count-1);
	strcpy(st_pthread[count-1].file_name, file_name);
	tfp = fopen(file_name, "w");
	while (!feof(fp_tmp))
	{
		read_length = fread(buffer, sizeof(char), read_size, fp_tmp);
		fwrite(buffer, sizeof(char), read_length, tfp);
	}
	fclose(tfp);

	fclose(fp_tmp);
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: ./%s ServerIPAddress\n",argv[0]);
		exit(1);
	}
	server_IP = argv[1];

	pthread_t child_thread[5];
	/*
	   pthread_attr_t child_thread_attr;
	   pthread_attr_init(&child_thread_attr);
	   pthread_attr_setdetachstate(&child_thread_attr,PTHREAD_CREATE_DETACHED);
	 */
	char file_name[BUFFER_SIZE];
	strcpy(file_name, "original_file");
	FILE *fp_original = fopen(file_name, "r");
	if (fp_original == NULL)
	{
		printf("not found file: %s\n", file_name);
		exit(-1);
	}

	struct st_pthread_type st_pthread[5];

	file_parting(fp_original, st_pthread);

	int i=0;
	for(i=0; i<5; ++i)
	{
		st_pthread[i].i = i;
		/*if( pthread_create(&(child_thread[i]), &child_thread_attr,talk_to_server, (void *)i) < 0 )*/
		if( pthread_create(&(child_thread[i]), NULL, talk_to_server, (void *)&st_pthread[i]) < 0 )
			printf("pthread_create Failed : %s\n",strerror(errno));
	}

	for (i=0; i<5; ++i)
	{
		pthread_join(child_thread[i], NULL);
	}

	return 0;

}
