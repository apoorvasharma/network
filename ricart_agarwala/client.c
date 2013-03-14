
/*
    This file is part of "Ricart Agarwala C Code".

    "Ricart Agarwala C Code" is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    "Ricart Agarwala C Code" is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.



*/



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
	char c;



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
		scanf("%c",&c);
		ra_lock(devices,SELF);
		printf("Fetching Res ID\n");
		res_fd = get_res_fd();
		sprintf(str,"Msg from %d\n",SELF);
		write(res_fd,str,strlen(str));
		close(res_fd);
		
		ra_unlock(SELF);
		usleep(500000);
		
		
	}

}
