#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>

typedef struct node node;
struct node{
	node *next;
	int score;    
	char *string;
        char* uid;
};

node *list_head=NULL;


void printdata ();
void print_list (node *list);
void update (char*id,char *newstring,int newscore);

//update linked list
void update (char*id,char *newstring,int newscore)
{
	
        // if list is empty, add new node in list
	if(list_head==NULL)
	{
		list_head=calloc(1,sizeof(node));
		list_head->string=calloc(1,strlen(newstring));
		strcpy(list_head->string,newstring);
                list_head->uid=calloc(1,strlen(id));
		strcpy(list_head->uid,id);
		list_head->next=NULL;
		list_head->score=newscore;
                
               

	}

        // if list is not empty and topic is same, generate new score
	else
	{ 	node *iter=list_head;
		
		while (!strcmp(iter->uid,id)) 
		{	
			if(strcmp(iter->string,newstring)==0)
			{

				iter->score+=newscore;
				break;
			} 
			else if(iter->next!=NULL)
			{

				iter=iter->next;	
			}
			else
			{

				iter->next=calloc(1,sizeof(node));
                                iter->next->uid=calloc(1,strlen(id));
                                strcpy(iter->next->uid,id);
				iter->next->string=calloc(1,strlen(newstring));
				strcpy(iter->next->string,newstring);
				iter->next->score=newscore;
                                iter->next->next=NULL;
                                
				break; 
			}

		} 
	}
}

//print the linked list and free memory
void print_list (node *list)
{	if (list!=NULL)
	{       
                
		printf("(%s,%s,%d) \n",list->uid,list->string,list->score);
		if (list->next!=NULL)
		{	
			print_list(list->next);
			free(list->next);
	 		list->next=0;
		}
                free(list->uid);
	 	free(list->string);
		list->next=0;
	} 

}

void printdata (){
	
	print_list(list_head);
	free(list_head);
	list_head=0;
}

void main()
{       
        char element[26] ="";
	char *id=malloc(5);
        char *subject=malloc(16);
	int  givenscore;
	char* previd =malloc(5);
        int ret;
		

	while(1){     
                       //get data from std input
                       fgets(element,100,stdin);

                       //process data to get required format while data is not newline character
                       if(strcmp(element,"\n"))
                       {char *tuple = element;
                        tuple++;
                        tuple[strlen(tuple)-2]='\0';
                        int j=0;
                       
                        //separate input into tokens                 
                        id = strtok(tuple,",");
                        
                        subject= strtok(NULL,",");
                        
                        givenscore= atoi(strtok(NULL,","));
                        
                         
                        //if UserId hasnt changed update linked list
			if (!strcmp(id,previd))
			{  
                          update(id,subject,givenscore);
                        }
                        // if user id changed, print previous id list 
			else
			{	
                                			
				printdata();
				update (id,subject,givenscore);
                                strcpy(previd,id);
			}
                     }
 else{printdata();break;}

}			
		 
	printdata();
	exit(0);
}

