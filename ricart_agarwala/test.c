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
