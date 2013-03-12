#include<stdio.h>
#include"queue.h"
#include<semaphore.h>

#define N 3
#define ADD_LEN 25
#define MSG_LEN 100
#define PORT 25000
#define USEC 100000
#define SEP ":"
#define REQ "REQ"
#define ACK "ACK"
#define DONT_WANT 0
#define WANT 1
#define HAVE 2
#define END "."
#define RECORD 0
#define CHANGE 1

//void* echo(void * data);


struct info
{
	int fd;
	int self;
	int num;
	
};


int strcnt(char *s,char c)
{
	int i;
	int ans = 0;
	for (i=0; s[i];i++) 
	{
		if(s[i] == c )
			ans++;
	}
	return ans;
}

pthread_mutex_t timer_lock,state_lock,queue_lock,num_lock;
sem_t res_sem;

int global_time = 0;
int global_state = DONT_WANT;
int global_ack = 0;
queue pending_list = NULL;





/*void timer(void* x)
{
	while(1)
	{
		pthread_mutex_lock(&timer_lock);
		global_time ++ ;
		//printf("signal %d\n",global_time);
		pthread_mutex_unlock(&timer_lock);
		//printf("beep\n");
		usleep(USEC);
	}	

}*/

void lamport_time(int t,int action)
{
	static int msg_time = 0;
	int gt;
	pthread_mutex_lock(&timer_lock);
	if(action == RECORD )
	{
		if(t/10 > msg_time)
		{
			msg_time = t/10 + 1; 
		
		
		}
		msg_time = global_time;
		
	}
	if(action == CHANGE )
	{
		global_time = msg_time;
	}
	pthread_mutex_unlock(&timer_lock);

	

}
void inc_time()
{

	pthread_mutex_lock(&timer_lock);
	global_time++;

	pthread_mutex_unlock(&timer_lock);

}
int get_time(int self)
{
	int t;
	pthread_mutex_lock(&timer_lock);
	t = global_time;
	//printf("self = %d,time = %d,total = %d\n",self,global_time,t*10 + self);
	pthread_mutex_unlock(&timer_lock);
	return (t*10 + self);
	//return 0;
}
/*
int get_msg_time(char msg[MSG_LEN])
{
	char copy[MSG_LEN];
	strcpy(copy,msg);
	return atoi(strtok(copy,":"));
}

char* get_msg_text(char msg[MSG_LEN])
{
	char copy[MSG_LEN];
	strcpy(copy,msg);
	strtok(copy,":");
	return strtok(NULL,":");
}
*/


void init()
{
	
//########################### Setting Alarm
	
	
	//lamport_time(1000);
//########################### Setting Mutex
	pthread_mutex_init(&timer_lock, NULL);
	pthread_mutex_init(&state_lock, NULL);
	pthread_mutex_init(&queue_lock, NULL);
	pthread_mutex_init(&num_lock, NULL);
//############################## Semaphore Init
	sem_init(&res_sem,0,0);
//################### starting timer
	
	
}

/*void timer_init()
{
	static pthread_t t;
	//pthread_create(&t, NULL,timer, NULL);
}*/


char  read_file(char* name,char addrs[N][ADD_LEN] ,int ports[N])
{

	int i;
	FILE* file = fopen(name,"r");
	char str[100];
	
	for(i=0;i<N;i++)
	{
		fscanf(file, "%s", str);
		strcpy(addrs[i],str);
		fscanf(file, "%s", str);
		ports[i] = atoi(str);
	}

	fclose(file);
}

void get_server_fd(int fd[N],int num,char addrs[N][ADD_LEN] ,int port)
{

	int listenfd,connfd;
	int i;
	struct sockaddr_in serv_addr;
   
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero( &serv_addr, sizeof(serv_addr));
     

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 
	
	for(i=0;i<num;i++)
	{

		
    	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		printf("Device %d Accepted\n",i);
		fd[i] = connfd;

	}
	printf("Server Setup done\n");
}

void get_client_fd(int fd[N],int num,char addrs[N][ADD_LEN] ,int port[N])
{

	int sockfd = 0;

    struct sockaddr_in serv_addr; 

	int result,i;
	struct timeval t;
	fd_set null_fd;
	
	t.tv_sec = 1;

	FD_ZERO(&null_fd);

	for(i=N-1;i > num;i--)
	{
		memset(&serv_addr, '0', sizeof(serv_addr)); 

    	serv_addr.sin_family = AF_INET;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		serv_addr.sin_port = htons(port[i]); 
		
		inet_pton(AF_INET, addrs[i], &serv_addr.sin_addr);
		
		while( 1)
		{
			result = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) ;
			if(result >= 0 )
				break;
			printf("Cant connect to device %d\n ",i);
			t.tv_sec = 1;
			t.tv_usec = 0;
			
			select(0,&null_fd,NULL,NULL,&t);
			
		}
		fd[i] = sockfd;
		printf("Device %d Connected\n",i);

		
		
	}

	printf("Client Setup done\n");

  

}



void send_msg(int fd, char * str,int self)
{
	int t;
	t = get_time(self);
	char send[MSG_LEN];

	sprintf(send,"%06d %s ",t,str);
	printf("********* Sending - %06d %s \n",t,str);
	write(fd,send,strlen(send));
}


void send_to_all(int*fd,char * msg,int self)
{
	int i;
	//printf("********SEND ALL : %s\n",msg);
	for(i=0;i<N;i++)	
	{
		if(fd[i] != -1 )
		{
			send_msg(fd[i],msg,self);
			//write(fd[i],msg,strlen(msg));
		}
		
	}
	inc_time();
}

void* echo(void * data)
{

	struct info * my_info = (struct info*)data;
	char rec[MSG_LEN];
	char send[MSG_LEN];
	char copy[MSG_LEN];
	char text[MSG_LEN];	
	int my_time,msg_time;
	int i,cnt;
	int offset;

	printf("Responding to Device %d \n",my_info->num);

	while(1)
	{
		bzero(rec,MSG_LEN);
		//printf("waiting.........\n");
		read(my_info->fd,rec,MSG_LEN);
		
		

		
		

		printf("********Raw Message - %s\n",rec);

	

		
		//msg_time = get_msg_time(rec);
		//strcpy(text,get_msg_text(rec));
		//printf("MSG from %d saying %s @ %d\n",my_info->num,text,get_msg_time(rec));
	
		offset = 0;
		pthread_mutex_lock(&state_lock);
		pthread_mutex_lock(&queue_lock);
		while(sscanf(&rec[offset],"%d %s",&msg_time,&text) != EOF)
		{


			//lamport_time(msg_time);
			my_time = get_time(my_info->self);
			printf("MSG from %d saying %s @ %d\n",my_info->num,text,msg_time);
			if(strcmp(text,REQ) == 0 )
			{
				if(global_state == DONT_WANT )
				{
					send_msg(my_info->fd,ACK,my_info->self);
					//printf("DONT WANT - Sending ACK to %d\n",my_info->num);
				}			
				else if(global_state == WANT )
				{
			
			
			
					if(msg_time < my_time)
					{
						send_msg(my_info->fd,ACK,my_info->self);
						printf("LESS PREF - Sending ACK to %d\n",my_info->num);
						//printf("MT - %d , OT - %d\n",my_time,msg_time);
					}
					else
					{
				
						pending_list = insert(pending_list,my_info->fd);	
						printf("MORE PREF - Deferring to %d\n",my_info->num);
					}
				}
				else if (global_state == HAVE )
				{
					pending_list = insert(pending_list,my_info->fd);	
					//printf("HAVE RES - Deferring to %d\n",my_info->num);
				}

			}	


			if(strcmp(text,ACK) == 0 )
			{
				pthread_mutex_lock(&num_lock);
				global_ack++;
				printf("ACK Number = %d\n",global_ack);

				if( global_ack == (N-1) )
				{
					printf("Acquiring \n");
					global_state = HAVE;
					sem_post(&res_sem);

				}
				pthread_mutex_unlock(&num_lock);
				lamport_time(msg_time,RECORD);
		
			}
			offset = offset + 11;
			
			//inc_time();
			
		}

		pthread_mutex_unlock(&state_lock);
		pthread_mutex_unlock(&queue_lock);
		//break;

		
		//write(my_info->fd,send,strlen(send));
		//sprintf("str = %s\n",str);
		//sleep(1);
	}
}


void start_comm_threads(int fds[N],int self)
{
	int i;
	static struct info info_array[N];
	pthread_t thread[N];
	for(i=0;i<N;i++)
	{
		if(i!=self)
		{
			//printf("i = %d fd = %d \n",i,fds[i]);
			
			
			info_array[i].self = self;
			info_array[i].fd = fds[i];
			info_array[i].num = i;
			pthread_create(&thread[i], NULL, echo, &info_array[i]);

		}
	}
}


int get_res_fd()
{
	
	int sockfd = 0;

    struct sockaddr_in serv_addr; 

	int result,i;



	memset(&serv_addr, '0', sizeof(serv_addr)); 

   	serv_addr.sin_family = AF_INET;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_port = htons(RES_PORT); 
		
	inet_pton(AF_INET, RES_ADD, &serv_addr.sin_addr);
		
	
	result = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) ;

	if(result == -1 )
	{
		printf("Fatal Error : Resource Not Found\n");
		exit(0);
	}
	return sockfd;

		
		
}

ra_lock(int* fds,int self)
{

	
	pthread_mutex_lock(&state_lock);
	printf("Locking\n");
	global_state = WANT;
	pthread_mutex_unlock(&state_lock);
	send_to_all(fds,REQ,self);

	sem_wait(&res_sem);
	
	
	/*
	int i;
	char rec[MSG_LEN];
	char text[MSG_LEN];


	
	for(i=0;i<N-1;i++)
	{
		if(fds[i] != -1 )
		{
			while(1)
			{
				read(fds[i],rec,MSG_LEN);
				strcpy(text,get_msg_text(rec));
				if( strcmp(text,ACK) == 0 )
				{
					printf("Received ACK from %d\n",i);
				}
			}	
		}
	}
*/
	printf("Locked\n");
}
		
ra_unlock(int self)
{
	int x;
	pthread_mutex_lock(&queue_lock);
	
	printf("Unlocking\n");
	while(pending_list!=NULL)
	{
		pending_list = del(pending_list,&x);
		send_msg(x,ACK,self);
		printf("Deleted %d from Queue\n",x);
	}

	pthread_mutex_unlock(&queue_lock);
	
	pthread_mutex_lock(&state_lock);
	global_state = DONT_WANT;
	pthread_mutex_unlock(&state_lock);


	pthread_mutex_lock(&num_lock);
	global_ack = 0;


	pthread_mutex_unlock(&num_lock);


	//sem_wait(&res_sem);
}

