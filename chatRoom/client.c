#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<sqlite3.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#define Port 8888

struct msg_node
{
    int id;
    char name[20];
    char pass[16];
	char msg[1000];
	char file[1000];
    int action;
	int conn_id;
};
struct threadinfo
{
  struct msg_node mssage;
  int sock;
};
////////////////////////////////////////////////////
int k_socket()
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        perror("Socket is error");
        return -1;
    }
    return sockfd;

}
int k_connect(int socketfd,char *argv)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv);
    servaddr.sin_port = htons(Port);

    int ret = connect(socketfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    if(ret == -1)
	{
		perror("connect is error");
		return -1;
	}
	return ret;

}
int sendfile(int sockfd,struct msg_node *msg_text)
{
int fp;
printf("please input accept file id\n");
scanf("%d",&(msg_text->conn_id));
fp = open("testclient.txt",O_RDWR,0755);
if(fp == -1)
{
	perror("open textcli error");
	exit(-1);
}
memset(msg_text->file,0,sizeof(msg_text->file));
int ret = read(fp,msg_text->file,1000);
if(ret == -1)
{
	perror("read error");
	exit(-1);
}
//printf("flie   %s\n",msg_text->file);
if(write(sockfd,msg_text,sizeof(*msg_text)) == -1)
{
	perror("write testcli error");
	exit(-1);
}
close(fp);
}
int online_f(int sockfd)
{
	char buff[1000];
	printf("wait......\n");
	int ret = read(sockfd,buff,1000);
	if(ret == -1)
	{
		perror("recv online_friend error");
		exit(-1);
	}
	printf("%s",buff);

}
int chat_query()
{
	FILE * fp= NULL;
	char ch1;
	if((fp = fopen("client.txt","r")) == NULL)
	{
		perror("fopen is error");
		pthread_exit(NULL);
	}
	while((ch1 = fgetc(fp)) != EOF)
	{
		printf("%c",ch1);
	}
	fclose(fp);
}
void *write_thread(void * arg)
{
	int sockfd;
	struct threadinfo FdandMs;
	FdandMs = *((struct threadinfo *)arg);
	sockfd = FdandMs.sock;
	struct msg_node msg_text;
	msg_text = FdandMs.mssage;
	char buff[1500];
	int fp;
	// scanf("%d",&(msg_text.action));
		
		int flag = 1;
		while (flag==1)
		{
			printf("please choice:\n");
		   printf("3.  say to all\n");
		   printf("4.  say to one\n");
		   printf("5.   send file\n");
		   printf("6.Online friends\n");
		   printf("9.Query your chat history\n");
		   if(msg_text.id == 0)
		   {
			   printf("7.Silence someone\n");
			   printf("8.Kick someone out\n");
		   }
		   scanf("%d",&(msg_text.action));
			switch (msg_text.action)
			{
			case 3:
			 if((fp =open("client.txt",O_CREAT | O_RDWR,0755)) == -1)
             {
                      perror("open error!\n");
                       exit(1);
             }
			         while (1)
	                {
						memset(buff,0,sizeof(buff));
		                  printf("please input massge\n");
						  printf("输入exit退出\n");
						  memset(msg_text.msg,0,sizeof(msg_text.msg));
		                  scanf("%s",msg_text.msg);
						  if(strcmp(msg_text.msg,"exit") == 0)
						  {
							  break;
						  }
						  sprintf(buff,"%d to all :%s",msg_text.id,msg_text.msg);
					       strcat(buff,"\n");  
						  int len = strlen(buff);
						  lseek(fp,0,SEEK_END);
						 if( write(fp,buff,len) == -1)
						 {
							 perror("write client.txt error");
							 exit(-1);
						 }
		                  int ret = write(sockfd,&msg_text,sizeof(msg_text));
		                  if(ret == -1)
		                  {
			                   perror("write is error");
			                    pthread_exit(NULL);
		                   }
	                 }
					 close(fp);
			break;
			case 4:
			 if((fp =open("client.txt",O_CREAT | O_RDWR,0755)) == -1)
             {
                      perror("open error!\n");
                       exit(1);
             }
			   while (1)
	                {
						memset(buff,0,sizeof(buff));
		                  printf("please input massge\n");
						  printf("输入exit退出\n");
						  memset(msg_text.msg,0,sizeof(msg_text.msg));
		                  scanf("%s",msg_text.msg);
						 
						  if(strcmp(msg_text.msg,"exit") == 0)
						  {
							  break;
						  } 
						  printf("输入要联系的客户ID\n");
						  scanf("%d",&(msg_text.conn_id));
		                  int ret = write(sockfd,&msg_text,sizeof(msg_text));
						  
		                  if(ret == -1)
		                  {
			                   perror("write is error");
			                    pthread_exit(NULL);
		                   }
						   sprintf(buff,"%d to %d :%s",msg_text.id,msg_text.conn_id,msg_text.msg);
					       strcat(buff,"\n");  
						  int len = strlen(buff);
						  lseek(fp,0,SEEK_END);
						 if( write(fp,buff,len) == -1)
						 {
							 perror("write client.txt error");
							 exit(-1);
						 }
	                 }
					 close(fp);

			break;
			case 5:
			sendfile(sockfd,&msg_text);
			break;
			case 6:
			printf("好友列表:\n");
			write(sockfd,&msg_text,sizeof(msg_text));
			sleep(1);
			break;
			case 7:
			printf("please input you want silence id\n");
			scanf("%d",&(msg_text.conn_id));
			write(sockfd,&msg_text,sizeof(msg_text));
			break;
			case 8:
			printf("please input you want kick out id\n");
			scanf("%d",&(msg_text.conn_id));
			write(sockfd,&msg_text,sizeof(msg_text));
			break;
			case 9:
			chat_query();
			break;
			default:
			flag = 0;
				break;
			}
		}
	
	pthread_exit(NULL);
}

void * read_thread(void * arg)
{
	char recvline[1000];
	int sockfd;
	int ret;
	int fp;
	sockfd = *((int *)arg);
	// printf("6.Online friends\n");
	// struct msg_node msg_text;
	// msg_text = FdandMs.mssage;

    while(1)
	{
		memset(recvline,0,sizeof(recvline));
		ret = read(sockfd,recvline,1000);
		if(ret == -1)
		{
			perror("client is error");
			pthread_exit(NULL);
		}
		if(strcmp(recvline,"admin Kick out you")  == 0)
		{
			exit(0);
		}
        printf("%s\n",recvline);
		if(strcmp(recvline,"Actionissendfile") == 0)
		{
			ret = read(sockfd,recvline,1000);
			fp = open("textser.txt",O_CREAT|O_RDWR,0755);
			if(fp == -1)
			{
				perror("open file test server error");
				pthread_exit(NULL);
			}
			int len = strlen(recvline);
			if(write(fp,recvline,len) == -1)
			{
				perror("write file test server error");
				pthread_exit(NULL);
			}
			close(fp);
		}
    }

	pthread_exit(NULL);
}

int afterlog(int sockfd,struct msg_node *msg_text)
{
	int action,ret;
	pthread_t tid_read;
	pthread_t tid_write;
	struct threadinfo FdandMs;
	FdandMs.mssage = *msg_text;
	FdandMs.sock = sockfd;
	int connd = sockfd;
    // printf("please input shiliao qunliao \n");
    // scanf("%d",&(msg_text->action));
	ret = pthread_create(&tid_write,NULL,(void *)write_thread,(void *)(&FdandMs));

	ret = pthread_create(&tid_read,NULL,(void*)read_thread,(void*)(&connd));

	
	pthread_detach(tid_write);
    pthread_join(tid_read,NULL);
}
//登录函数
int logclient(int sockfd,struct msg_node *msg_text)
{
	int ret;
	char buff[100];
	int id;
	msg_text->action = 1;
	printf("please input your id\n");
	scanf("%d",&id);
	printf("please input your password\n");
	scanf("%s",msg_text->pass);
	msg_text->id = id;
	ret = write(sockfd,msg_text,sizeof(*msg_text));
	memset(buff,0,sizeof(buff));
	ret = recv(sockfd,buff,100,0);
	printf("%s",buff);
	if(strcmp(buff,"login success") == 0)
	{
		afterlog(sockfd,msg_text);
	}
	
}

//注册函数
int enroll(int sockfd,struct msg_node *msg_text)
{
	int ret;
	char buff[100];
	// int id;
	// printf("please input your id\n");
	// scanf("%d",&id);
	msg_text->action = 2;
	printf("please input your name\n");
	scanf("%s",msg_text->name);
		printf("please input your pass\n");
	scanf("%s",msg_text->pass);
	// msg_text->id = id;
	ret = write(sockfd,msg_text,sizeof(*msg_text));
	memset(buff,0,sizeof(buff));
	ret = recv(sockfd,buff,sizeof(buff),0);
	printf(" eroll buff %s",buff);
	//printf("id  = %d\n",msg_text->id);

}
//主体
void choice(int sockfd)
{
	struct msg_node msg_text;
	int logsta;
	int flag = 1;
	
	// while(flag == 1)
	// {
		printf("欢迎使用聊天室\n");
		printf("1.登录\n");
		printf("2.注册\n");
		printf("please input your choice\n");
	    scanf("%d",&msg_text.action);
	switch (msg_text.action)
	{
	case 1:
	logclient(sockfd,&msg_text);
	// flag == 0;
		
		break;
	case 2:
	enroll(sockfd,&msg_text);
	break;
	
	default:
	// flag = 0;
		break;
	}
	// }

}
int main(int argc,char **argv)
{
      int sockfd;
	int ret;
	pthread_t tid_read;
	pthread_t tid_write;
  //  char sendline[1000];
 //   char recvline[1000];
//	struct msg_node  msg_text;
//	int choice;

    if(argc != 2)
    {
        printf("usage:client <IP address>\n");
        exit(1);
    }
    sockfd = k_socket();
	if(k_connect(sockfd,argv[1]) == -1)
	{
		return 0;
	}
	
		choice(sockfd);
		

close(sockfd);

    return 0;
}