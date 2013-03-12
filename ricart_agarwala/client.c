/*
Client Program
Gets own device number through commandline and accesses resourced through 
acquires reource by calling ra_lock, sends data to server, waits for specifed
seconds and releases resource with ra_unlock
See client.h for function descriptions
*/
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

	char str[MSG_LEN];
	int SELF = atoi(argv[1]);

	/*array to hold file-descriptors of other devices*/
	int devices[N] = {0};


	/*arrays to hold addresses and port numbers of other devices*/
	char address[N][ADD_LEN];
	int port[N];




	int res_fd;


	/*device is not connected to itself*/
	devices[SELF] = -1;


	/*Mandatory Initializations*/
	init();
	read_file("address.txt",address,port);


	get_server_fd(devices,SELF,address,port[SELF]);
	get_client_fd(devices,SELF,address,port);
	start_comm_threads(devices,SELF);
	


	while(1)
	{

		/*Repeatedly trying to connect,access and disconnect */
		
		ra_lock(devices,SELF);
		printf("Fetching Res ID\n");
		res_fd = get_res_fd();
		sprintf(str,"Msg from %d\n",SELF);
		write(res_fd,str,strlen(str));
		close(res_fd);
		sleep(1);
		ra_unlock(SELF);
		
		
		
	}

}
