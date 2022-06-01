#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#define Port 8888
sqlite3 *pdb;

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
typedef struct online_client
{

    int id;
    int sockfd;
    int talk;
    char name[20];
    struct online_client *next;

} Online;
typedef Online *OnlineLink;
struct threadwork
{
    int connd;
    OnlineLink head;
};
void is_malloc_ok(OnlineLink new_node)
{
    if (NULL == new_node)
    {
        printf("malloc error\n");
        exit(-1);
    }
}
void create_new_node(OnlineLink *new_node)
{
    (*new_node) = (OnlineLink)malloc(sizeof(Online));
   
    is_malloc_ok(*new_node); 
    
    (*new_node)->talk = 1;
}
void createLink(OnlineLink *head)
{
    create_new_node(head);
    (*head)->next = NULL;
}
void insert_link(OnlineLink head, OnlineLink newnode)
{
    OnlineLink p = NULL;
    p = head;
    newnode->next = p->next;
    p->next = newnode;
}
int k_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("create sockfd is error\n");
        exit(1);
    }
    return sockfd;
}
int k_bind(int sockfd)
{
    struct sockaddr_in serveraddr;
    int length;
    int bsockfd = sockfd;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); // htonl(INADDR_ANY);
    serveraddr.sin_port = htons(Port);
    int opt = 1;
    // sockfd为需要端口复用的套接字
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("bind is error");
        return -1;
    }
    listen(bsockfd, 1024);
    return bsockfd;
}
int k_accept(int sockfd)
{
    struct sockaddr_in clientaddr;
    int length = sizeof(clientaddr);
    int connd = accept(sockfd, (struct sockaddr *)&clientaddr, &length);
    if (connd == -1)
    {
        perror("accept is error");
        return -1;
    }
    return connd;
}
int display(void *para, int columnCount, char **columValue, char **columnName)
{
    int i;
    for (i = 0; i < columnCount; i++)
    {
        printf("%10s", columnName[i]);
    }
    printf("\n");
    for (i = 0; i < columnCount; i++)
    {
        printf("%10s", columValue[i]);
    }
    printf("\n");
    return 0;
}
int select_table()
{
    char *sql = NULL;
    char *errmsg = NULL;
    sql = "select * from clientlist;";
    if (SQLITE_OK != sqlite3_exec(pdb, sql, display, NULL, &errmsg))
    {
        printf("select is error !%s", errmsg);
    }
}
int create_table()
{
    int ret = sqlite3_open("chatRoom.db", &pdb);
    if (SQLITE_OK != ret)
    {
        printf("open database error %s\n", sqlite3_errmsg(pdb));
        exit(-1);
    }
    char *sql = NULL;
    char *errmsg = NULL;
    sql = "create table if not exists clientlist(id integer primary key,name text,pass text);";
    if (SQLITE_OK != sqlite3_exec(pdb, sql, NULL, NULL, &errmsg))
    {
        perror("create table  is error ");
    }
}
int insert_table(struct msg_node *client_info)
{
    char sql[100];
    char *errmsg = NULL;
    printf("client_info id %d\n", client_info->id);
    sprintf(sql, "insert into clientlist(id,name,pass) values(%d,'%s','%s');", client_info->id, client_info->name, client_info->pass);
    printf("%d   %s    %s", client_info->id, client_info->name, client_info->pass);
    if (SQLITE_OK != sqlite3_exec(pdb, sql, NULL, NULL, &errmsg))
    {
        perror("insert table is error!");
        return -1;
    }
    return 0;
}
int search_table( struct msg_node *client)
{
    char sql[100];
    char * errmsg;
     char ** result;        
     char pass[16];
    int row = 0,column = 0;
    printf("\n11111111111111111111\n");
    sprintf(sql,"select * from clientlist where id=%d;",client->id);
    
     if(SQLITE_OK != sqlite3_get_table(pdb,sql,&result,&row,&column,&errmsg))
   {
       printf("check out error!  %s\n",errmsg);     
   } 

   
    if(row!=0&&column!=0)
    {
          strcpy(pass,result[2*column-1]); 
          strcpy(client->name,result[2*column-2]);
           if(strcmp(pass,client->pass) == 0)
          {
                 return 0;
           }
           return -2;
    }
    
    return -1;
    

}
void release_link(Online **head)
{
    Online *temp;
    temp = *head;

    while (*head != NULL)
    {
        *head = temp->next;
        free(temp);
        temp = *head;
    }
}
void display_link(OnlineLink head)
{
    OnlineLink p = NULL;
    if (NULL == head)
    {
        printf("no such node\n");
        return;
    }
    p = head->next;
    if (NULL == p)
    {
        printf("Link is empty\n");
        return;
    }
    while (NULL != p)
    {
        printf("id:%d name:%s  talk:%d", p->id,p->name,p->talk);
        p = p->next;
    }
    printf("\n");
}
void release_node(int connfd, OnlineLink client_llink)
{
    // Online *head;
    Online *p1 = NULL;
    Online *p2 = NULL;

    // head =

    p1 = client_llink->next;
    p2 = client_llink;

    if (p1 == NULL)
    {
        printf("Link is empty!\n");
    }
    else
    {
        while (p1->next != NULL && p1->sockfd != connfd)
        {
            p2 = p1;
            p1 = p1->next;
        }

        if (p1->next != NULL)
        {
            p2->next = p1->next;
            free(p1);
        }
        else
        {
            if (p1->sockfd == connfd)
            {
                p2->next = NULL;
                free(p1);
            }
            else
            {
                printf("not node is attach!\n");
            }
        }
    }
}
int enroll(int connfd, struct msg_node *client)
{
    // struct msg_node client;
    char buff[1000];
    srand(time(NULL));
    // int ret = recv(connfd,&client,sizeof(client),0);
    int id;
    client->id = rand() % 10000;
    int ret = insert_table(client);
    memset(buff,0,sizeof(buff));
    if (ret == 0)
    {
        printf("\nsheng cheng id %d", client->id);
        sprintf(buff, "enroll is sucess id is %d", client->id);
        ret = write(connfd, buff, sizeof(buff));
    }
}
int client_login(int connfd, OnlineLink *head, struct msg_node *client)
{
    // struct msg_node client;
    OnlineLink new_node;
    char buff[100];
    int exist_cli = search_table(client);
    int ret;
    printf("exisT_cli  %d\n",exist_cli);
    if(exist_cli == -1)
    {
         strcpy(buff, "CLIENT NO EXIST");
     ret = write(connfd, buff, sizeof(buff));
    if (ret == -1)
    {
        perror("server write to client error");
        pthread_exit(NULL);
    }
    return -1;
    }
      if(exist_cli == -2)
    {
         strcpy(buff, "pass word error");
     ret = write(connfd, buff, sizeof(buff));
    if (ret == -1)
    {
        perror("server write to client error");
       pthread_exit(NULL);
    }
    return -1;
    }
    create_new_node(&new_node);
    new_node->id = client->id;
    new_node->sockfd = connfd;
    strcpy(new_node->name,client->name);
    insert_link(*head, new_node);
    printf("id = %d\n", client->id);
    memset(buff,0,100);
    strcpy(buff, "login success");
     ret = write(connfd, buff, sizeof(buff));
    if (ret == -1)
    {
        perror("server write to client error");
        pthread_exit(NULL);
    }
    return 0;
}
int sendtoall(int sockfd,OnlineLink clientlink,struct msg_node msg_text)
{
    printf("massge  %s\n",msg_text.msg);
    int n;
    int fp1;
    Online * tmp = NULL;
    OnlineLink p = NULL;
    char buff[1500];

    tmp = clientlink->next;
    p = clientlink->next;
    while( p != NULL && p->sockfd != sockfd)
    {
        p = p->next;
    }
    if(p->talk == 1)
    {

	n = strlen(msg_text.msg);
     ////////////////打开保存全部聊天记录的文件  
    if((fp1 = open("server.txt",O_CREAT|O_RDWR,0755)) == -1)
    {
        perror("open server.txt error");
        pthread_exit(NULL);
    }
     	
    while(tmp != NULL)
   {
    //	sendto(connfd,msg,n,0,
	//			(struct sockaddr*)&cliaddr,sizeof(cliaddr));
        printf("sockfd   %d\n",tmp->sockfd);
        if(tmp->id != msg_text.id)
        {
           
            int b =  write(tmp->sockfd,msg_text.msg,n);
            if(b == -1)
             {
                     perror("WRITE IS ERROR");
                     pthread_exit(NULL);
             }
        }
	memset(buff,0,sizeof(buff));
     sprintf(buff,"%d toall :%s",msg_text.id,msg_text.msg);
	strcat(buff,"\n");  
	int len = strlen(buff);
	lseek(fp1,0,SEEK_END);
	 if( write(fp1,buff,len) == -1)
	{
		perror("write client.txt error");
		pthread_exit(NULL);
	 }
        tmp = tmp->next;
   }
  
     close(fp1);
    }
    else
    {
        memset(buff,0,sizeof(buff)); 
        strcpy(buff,"Silence");
        int length =  strlen(buff);
        send(sockfd,buff,length,0);
    }
    


}
//私聊
int sendtoone(int sockfd,OnlineLink clientlink,struct msg_node msg_text)
{
    int n;
    char buff[1500];
    int fp;
    OnlineLink p =NULL;
      p = clientlink->next;
    while( p != NULL && p->sockfd != sockfd)
    {
        p = p->next;
    }
    if(p->talk == 1)
    {
    strcpy(buff,msg_text.msg);
    printf("%s  to %d\n",msg_text.msg,msg_text.conn_id);

    Online * tmp;

    tmp =clientlink->next;
         	
    n = strlen(buff);
    if((fp = open("server.txt",O_CREAT|O_RDWR,0755)) == -1)
    {
        perror("open server.txt(2) error");
        pthread_exit(NULL);
    }

    while(tmp != NULL && tmp->id != msg_text.conn_id)
   {
    //	sendto(connfd,msg,n,0,
	//			(struct sockaddr*)&cliaddr,sizeof(cliaddr));
	  //  send(tmp->connfd,msg,n,0);
        tmp = tmp->next;
   }

	if(tmp != NULL)
	{
		send(tmp->sockfd,buff,n,0);
	}
	else
	{
        memset(buff,0,sizeof(buff));
		strcpy(buff,"client is offline!");
        n = strlen(buff);
        send(sockfd,buff,n,0);
	}
    memset(buff,0,sizeof(buff));
    sprintf(buff,"%d to %d :%s",msg_text.id,msg_text.conn_id,msg_text.msg);
	strcat(buff,"\n");  
	int len = strlen(buff);
	lseek(fp,0,SEEK_END);
	 if( write(fp,buff,len) == -1)
	{
		 perror("write client.txt error");
		pthread_exit(NULL);
	}

    close(fp);
    }
     else
    {
        memset(buff,0,sizeof(buff)); 
        strcpy(buff,"Silence");
        int length =  strlen(buff);
        send(sockfd,buff,length,0);
    }
    

}
int online_f(int sockfd,OnlineLink clientlink)
{
    char buff[500];
    char friend[500];
    memset(friend,0,500);
    OnlineLink p = NULL;

    p = clientlink->next;
 
    if(p==NULL)
    {
        strcpy(buff,"No online friends");
    }
    else
    {
        while(p != NULL)
        {
            memset(buff,0,sizeof(buff));
            sprintf(buff,"%s(id:%d)\n",p->name,p->id);
            int n = strlen(buff);
            strncat(friend,buff,n);
            p = p->next;
        }
        int len = strlen(friend);
        printf("friend :  %s\n",friend);
     
        int ret = send(sockfd,friend,len,0);
       
    }
}
int silence(int sockfd,OnlineLink clientlink,struct msg_node msg_text)
{
    OnlineLink p = NULL;
    char buff[30];
    p = clientlink->next;
    while(p !=NULL&& p->id != msg_text.conn_id)
    {
        p = p->next;
    }
    if(p == NULL)
    {
        strcpy(buff,"client no online\n");
        send(sockfd,buff,sizeof(buff),0);
        return -1;
    }
    p->talk = 0;
    return 0;

}
int kick_out(int sockfd,OnlineLink clientlink,struct msg_node msg_text)
{
    OnlineLink p =NULL;
    p = clientlink->next;
    char buff[30];
    while(p != NULL&& p->id != msg_text.conn_id)
    {
        p = p->next;
    }
     if(p == NULL)
    {
        strcpy(buff,"client no online\n");
        send(sockfd,buff,sizeof(buff),0);
        return -1;
    }
  
   memset(buff,0,sizeof(buff));
   strcpy(buff,"admin Kick out you");
  if( send(p->sockfd,buff,sizeof(buff),0) == -1)
  {
      perror("send kick error");
      pthread_exit(NULL);
  } 
   release_node(p->sockfd,clientlink);
   return 0;

}
int send_file(int sockfd,OnlineLink clientlink,struct msg_node msg_text)
{
    int fp;
    OnlineLink p =NULL;
    p = clientlink->next;
    while(p != NULL&& p->id != msg_text.conn_id)
    {
        p = p->next;
    }
    if(p == NULL)
    {
        send(sockfd,"client offline",sizeof("client offline"),0);
        pthread_exit(NULL);
    }
    fp = open("textser.txt",O_CREAT|O_RDWR,0755);
    if(fp == -1)
    {
        perror("open sertxt error");
        pthread_exit(NULL);
    }
    int len = strlen(msg_text.file);
    printf("flie %s\n",msg_text.file);
    printf("connd_id   %d\n",msg_text.conn_id);
    char buff[50] = "Actionissendfile";
    write(p->sockfd,buff,sizeof(buff));
sleep(1);
    if(write(p->sockfd,msg_text.file,len)  == -1)
    {
        perror("write sertxt error");
        pthread_exit(NULL);
    }
    close(fp);

}
void choice(int connd, OnlineLink clientlink)
{
    struct msg_node msg_text;
    for(;;)
    {
       int ret = recv(connd, &msg_text, sizeof(msg_text), 0);
       if (ret == 0)
      {
          printf("client offline\n");\
          release_node(connd,clientlink);
            printf("--------clientOnline----------\n");
        display_link(clientlink);
        printf("\n--------------------------------\n");
          pthread_exit(NULL);
      }
      printf("action %d\n", msg_text.action);
      printf("id is %d\n", msg_text.id);
  
    switch (msg_text.action)
    {
    case 1:
        client_login(connd, &clientlink, &msg_text);
        break;
    case 2: 
        printf("name is %s\n", msg_text.name);
        enroll(connd, &msg_text);
       // printf("eroll 111111111111111\n");
        break;
   case 3:
        sendtoall(connd,clientlink,msg_text);
    break;
    case 4:
    sendtoone(connd,clientlink,msg_text);
    break;
    case 5:
      send_file(connd,clientlink,msg_text);
    break;
    case 6:
    online_f(connd,clientlink);
    break;
    case 7:
    silence(connd,clientlink,msg_text);
    break;
    case 8:
    kick_out(connd,clientlink,msg_text);
    break;
       
    default:
        break;
    }
      printf("------------------------\n");
        select_table();
        printf("\n------------------------\n");
        printf("--------clientOnline----------\n");
        display_link(clientlink);
        printf("\n--------------------------------\n");
        //online_f(connd,clientlink);
    }
    close(connd);
}

void pthread_func(void * arg)
{
       struct threadwork connd_node;
       connd_node = *((struct threadwork * )arg);
       int connd = connd_node.connd;
       
        //choice(connd, clientlink);
        choice(connd, connd_node.head);
      

}
int main()
{
    int sock_fd = k_socket();
    pthread_t pid;
    int ret;
    OnlineLink clientlink;
    OnlineLink newnode;
    createLink(&clientlink);
    create_table(pdb);
    int login;
    struct threadwork connd_node;
    connd_node.head = clientlink;
    if (sock_fd == -1)
    {
        return -1;
    }
    if (k_bind(sock_fd) == -1)
    {
        return -1;
    }
    printf("Wainting client connect server.......\n");

    while (1)
    {
        int connd = k_accept(sock_fd);
        if (connd == -1)
        {
            continue;
        }
        connd_node.connd = connd;

        ret = pthread_create(&pid,NULL,(void *)pthread_func,(void*)&connd_node);
        if(ret == -1)
        {
            perror("thread is error");
            exit(-1);
        }
    }
    // display_link(clientlink);
    close(sock_fd);
    release_link(&clientlink);
    sqlite3_close(pdb);
    return 0;
}
