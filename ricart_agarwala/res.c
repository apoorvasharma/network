#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <time.h>
#include<string.h>
#include<signal.h>
#include<stdlib.h>
#include<errno.h>
#include "res.h"

#define MAX 1024
#define SA struct sockaddr

FILE* file;


void int_handle(int x)
{
	fclose(file);
	printf("Process Exiting\n");
	exit(0);
	//
}


int main(int argc, char **argv)
{
 	int listenfd, connfd;
	int status;
 	struct sockaddr_in servaddr;
	char buff[MAX];
	listenfd=socket(AF_INET, SOCK_STREAM, 0);
	
	bzero( &servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);////////////
	servaddr.sin_port = htons(RES_PORT);

	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	
	file = fopen("res.txt","w");

	signal(SIGINT,int_handle);
	listen(listenfd, 1);
	while(1) 
	{
    	connfd = accept(listenfd, (SA *) NULL, NULL);
		printf("Connected\n");

			while(1)
			{
				
				bzero( buff, MAX);   
				status = read(connfd,buff,MAX);
				    
        		if(status == 0)
				{
					close(connfd);
					break;
				}
				//printf("status %d error %d\n",status,errno);

				
				printf("Writing : %s\n",buff);
				fprintf(file,"%s",buff);
				

        		//write(connfd, buff, strlen(buff)+1);

			
			}
		
	
	}
}


