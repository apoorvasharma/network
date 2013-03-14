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
Queue Implementation for deferring messages while unlocking
*/
struct node_ 
{
	int data;
	struct node_ * next;

};

typedef struct node_ node;
typedef node* queue;

node* new_q()
{
	node * newnode;

	newnode->next = NULL;
}

node* insert(queue q,int x)
{
	node * start = q;


	node * newnode;
	node * current;

	newnode = ( node*)malloc(sizeof(node));	
	newnode->data = x;
	newnode->next = NULL;

	if(start == NULL )
	{
		
		start =  newnode;
		return start;
	}
	else
	{
		current  = start;
		while( (current->next) != NULL )
			current = current->next;
		current -> next = newnode;
		return start;
	}
	
}

node* del(queue q,int* x)
{
	node * temp = q;
	
	if(q == NULL )
	{
		printf("Error : Deleting from empty Queue");
		return NULL;
	}

	else
	{
		*x = q->data;
		temp = q->next;
		free(q);
		return temp;
		
	}

	

	
	
}





