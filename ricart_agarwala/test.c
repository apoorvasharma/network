#include<stdio.h>
#include<stdlib.h>


void main()
{
	int t;
	char str[100];
	char inp[100] = "REQ 1234 ACK 5678";
	int offset = 0;
	
	while(sscanf(&inp[offset],"%s %d",str,&t)!=EOF)
	{

		sleep(1);
		printf("****************************************************\n");
		printf("input = %s\n",&inp[offset]);		
		printf("%s %07d\n",str,t);
		offset = offset + 9;
		
		sleep(1);
		
	}
}
