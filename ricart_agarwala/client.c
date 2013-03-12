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
#include "res.h"
#include "client.h"






void main(int argc,char** argv)
{
	struct timeval tv;
	tv.tv_sec = 1;
	
	int SELF = atoi(argv[1]);
	int devices[N] = {0};
	int listen;
	int i;
	char address[N][ADD_LEN];
	int port[N];
	fd_set read_set;
	int read_num;
	char c;
	char str[MSG_LEN];
	char send[MSG_LEN],rec[MSG_LEN];
	int res_fd;
	int r;
    int listenfd ,connfd;
    struct sockaddr_in addr; 
	
	queue list = NULL;


	devices[SELF] = -1;

	init();

	//timer_init();


//############################ Reading Files
	read_file("address.txt",address,port);

//########################## SERVERS SET UP

	get_server_fd(devices,SELF,address,port[SELF]);
	get_client_fd(devices,SELF,address,port);
	start_comm_threads(devices,SELF);
	


	while(1)
	{
		//scanf("%c",&c);
		//r = rand()*1000/RAND_MAX;
		//usleep(r*1000);
		
		printf("*************************************************\n");
		
		ra_lock(devices,SELF);
		printf("Fetching Res ID\n");
		res_fd = get_res_fd();
		sprintf(str,"Msg from %d\n",SELF);
		write(res_fd,str,strlen(str));
		close(res_fd);
		sleep(1);
		ra_unlock(SELF);
		lamport_time(0,CHANGE);
		
		
	}
	/*ra_lock(devices);
	res_fd = get_res_fd();
	sprintf(str,"Msg from %d\n",SELF);
	write(res_fd,str,strlen(str));
	close(res_fd);
	ra_unlock();*/
	
	scanf("%c",&c);


}
