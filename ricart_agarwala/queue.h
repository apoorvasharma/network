
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





