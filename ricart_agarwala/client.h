/*
Each device has a logical number (< N)
For a TCP/IP connection it acts as a server for lower numbered devices
and acts as a client to higher numbered devices
Total connections = N*(N-1)/2
Each device has (N-1) threads for communicating simultaneousy with other devices
*/

/*Header containing required functions */


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <signal.h>
#include"queue.h"
#include<semaphore.h>


/* Nmmber of devices, known in advance and fixed*/
#define N 4


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



/*Information to be passes to each communicating thread*/
struct info
{
	int fd;
	int self;
	int num;
	
};



/*Global varibales and locks*/
pthread_mutex_t timer_lock,state_lock,queue_lock,num_lock;
sem_t res_sem;

int global_time = 0;
int global_state = DONT_WANT;
int global_ack = 0;
queue pending_list = NULL;



/*
Function to implement Lamports logical clock
when action = CHANGE , records 't' if it is greater 
when action = CHANGE , changes 'global_time'

Change this function for more than 10 devices
*/
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

/*
Increments time
Called during significant event ( sending message )

*/
void inc_time()
{

	pthread_mutex_lock(&timer_lock);
	global_time++;
	pthread_mutex_unlock(&timer_lock);

}

/*
Return the time, adjusted for priority
Done so that no 2 deviced share the same time stamp
*/
int get_time(int self)
{
	int t;
	pthread_mutex_lock(&timer_lock);
	t = global_time;
	pthread_mutex_unlock(&timer_lock);
	return (t*10 + self);
	
}



void init()
{
	

	pthread_mutex_init(&timer_lock, NULL);
	pthread_mutex_init(&state_lock, NULL);
	pthread_mutex_init(&queue_lock, NULL);
	pthread_mutex_init(&num_lock, NULL);

	sem_init(&res_sem,0,0);

	
	
}


/*Parse addresses and port numbers*/

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


/*
Wait for connections from lower numbered devices serially and put
file descriptors in 'fd'
*/
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



/*
Serially Connect to higher numbered Devices, keep retrying if not avaialable,
add file descriptors in 'fd'
*/
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


/*
Send a message with time-stamp
message length is fixed
*/
void send_msg(int fd, char * str,int self)
{
	int t;
	t = get_time(self);
	char send[MSG_LEN];

	sprintf(send,"%06d %s ",t,str);
	//printf("********* Sending - %06d %s \n",t,str);
	write(fd,send,strlen(send));
}

/*
Send to all devices in 'fd' array
*/
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


/*
Function which each of the (N-1) threads run
each thread gets data through struct info, which containt own logical number,
file descriptor, and connected device number

The Function is in an infinite loop to respond to messages accoring to Ricart-
Agarwala Algorithm, comparing timestamps

Comment out all the printfs to get detailed runtime output
*/
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
		
		

		//printf("********Raw Message - %s\n",rec);

	

		
		//msg_time = get_msg_time(rec);
		//strcpy(text,get_msg_text(rec));
		//printf("MSG from %d saying %s @ %d\n",my_info->num,text,get_msg_time(rec));
	
		offset = 0;
		pthread_mutex_lock(&state_lock);
		pthread_mutex_lock(&queue_lock);

		/*
		Message Queue might contain more than one arrived messages
		below loop examines all of them
		*/
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
						//printf("LESS PREF - Sending ACK to %d\n",my_info->num);
						//printf("MT - %d , OT - %d\n",my_time,msg_time);
					}
					else
					{
				
						pending_list = insert(pending_list,my_info->fd);	
						//printf("MORE PREF - Deferring to %d\n",my_info->num);
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
				//printf("ACK Number = %d\n",global_ack);

				if( global_ack == (N-1) )
				{
					printf("Acquiring \n");
					global_state = HAVE;

					/*so that ra_lock can proceed */
					sem_post(&res_sem);


				}
				pthread_mutex_unlock(&num_lock);

				/*
				time is only recorded and not updated
				This is done so that all the messages in a single locking 
				cycle contain the same time stamp
				*/
				lamport_time(msg_time,RECORD);
		
			}
			offset = offset + 11;
			

			
		}

		pthread_mutex_unlock(&state_lock);
		pthread_mutex_unlock(&queue_lock);
		//break;

		
		//write(my_info->fd,send,strlen(send));
		//sprintf("str = %s\n",str);
		//sleep(1);
	}
}

/*
Start threads for all other devices
*/
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

/*
Connect and get critical resource's file descriptor
*/
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


/*
Function to acquire lock on resource
It send requests to all devices and waits due to semaphore 'res_sem'
The semaphore is incremented by 'echo' function when required ACKs are
receieved
*/
ra_lock(int* fds,int self)
{

	
	pthread_mutex_lock(&state_lock);
	printf("Locking\n");
	global_state = WANT;
	pthread_mutex_unlock(&state_lock);
	send_to_all(fds,REQ,self);


	/*wait till all ACKs arrive */
	sem_wait(&res_sem);
	
	

	printf("Locked\n");
}

/*
Unlock the resource
*/
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

	/*Change time according to all messages received since last change */
	lamport_time(0,CHANGE);
}

