#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

static sem_t sem;
static sem_t full;
static sem_t empty;
int over ;
int numberprod = 0;
int numbercons = 0;

typedef struct basic_tup{
	char ** tuple_ptr;
	char id[5];
	int itemid;
}shared_tup;

shared_tup buff[50];

typedef struct rbuf{
	char ** tuple;
	int top;
	int score[1000];
	char usrid[5];
}outbuf;

outbuf outbuffer[50];

int bufsize;
int numred;
//void *mapper_thread(void *thid);
int strlookup(char temparr[30]);
//void *reducer_thread(void *thid);
char * scoreis(char top);
//returns the corresponding val based on the topic.





char * scoreis(char top)
{
    static char *val;

    switch(top)
   {
     case 'S' : val ="40";
                break;
     
     case 'P' : val ="50";
                break;
     
     case 'L' : val= "20";
                break;
     
     case 'D' : val="-10";
                break;
     
     case 'C' : val="30";
                break;
   
     default  : val=NULL;

   }
    
 
    return val;
}

void *mapper_thread(void *thid){

	long thread_index = (long) thid;
	
	FILE *filepointer  = fopen("input.txt", "r"); // read only
    char top;
    over = 0;
	char temparr[30];
	char temparr_new[3];
	char index[3];
	int count_new=1;
	char newtuple[26];
	char ch = fgetc(filepointer);
    if (filepointer == NULL)
        {
              printf("Error! printfCould not open file\n");
              exit(-1); // must include stdlib.h
        }
	int integer=0;
	fseek(filepointer, 0, SEEK_SET);

	while(!feof(filepointer))
	{
		ch = fgetc(filepointer);
		if (ch == ')')
		{
			integer++;
		}
	}

	fseek(filepointer, 0, SEEK_SET);

    //mapping program	
    for(int j=0; j<=integer; j++)
    {
    	int position=0;
		while(1)
		{
			ch=fgetc(filepointer);
			if (ch == '(')
			{
				break;
			}
		}
		fread(temparr, 22, 1, filepointer);
    	for(int i=0; i<=23; i++)
    	{
       		if(i>=0 && i<4)
			{
				newtuple[i+1]=temparr[i];
				index[i]=temparr[i];	
			}
			if(i==5)
			{
				top=temparr[i];	
			}
			if(i>=7 && i<=22)
			{
				newtuple[i-1]=temparr[i];	
			}		
		}
		newtuple[0]='(';
		newtuple[5]=',';
		newtuple[21]=',';
		strcpy(temparr_new, scoreis(top));
		for(int j=0; j<3; j++)
		{
			newtuple[22+j]=temparr_new[j];
		}
		if (temparr_new[0]=='-')
		{
			newtuple[25]=')';
			newtuple[26]='\0';
		}
		else
		{
			newtuple[24]=')';
			newtuple[25]='\0';
		}
		index[4]='\0';
		

		//code actually starts from here
		if (j==integer-1)
		{
			over=1;
		}
		sem_wait(&sem);
		if (over) {
       		
       		sem_post(&empty);	
		sem_post(&sem); 
       		return NULL;
    	}
		for(int a=0; a<numred; a++){
			if(strcmp(index, buff[a].id)==0){
				position=a;
				break;
			}
		}
		while (buff[position].itemid==bufsize) { 
      		
      		sem_post(&sem);
		sem_wait(&full);
		sem_wait(&sem);
    	} 
		for(int k=0; k<numred; k++){
			if(strcmp(index,buff[k].id)!=0){
				if(buff[k].id[0]=='\0'){
					strcpy(buff[k].id,index);
					break;
				}
				continue;
			}
			else
			{
				break;
			}
		}

		for(int w=0; w<numred; w++){
			if(strcmp(index, buff[w].id)==0){
				for(int e=0; e<bufsize; e++){
					if(buff[w].tuple_ptr[e][0]=='\0'){
						strcpy(buff[w].tuple_ptr[e],newtuple);
						numberprod++;
						buff[w].itemid++;
						
						break;
					}
				}
				break;
			}
		}
		sem_post(&sem);
		sem_post(&empty);		
    }
    fclose(filepointer);
    over=1;
    return NULL;
}

//reducer

int strlookup(char temparr[30])
{
	for(int i=0; i<strlen(temparr); i++)
	{ 
		if(temparr[i]=='-')
		{
			return i;
		}
	}
	return 0;
}
 

void *reducer_thread(void *thid)
{
	long thread_index = (long)thid;
	
    //reducer program	
    while(1)
    {			
    			sem_wait(&sem);
				while (buff[thread_index-1].itemid==0 && !over) {
       				
       				sem_post(&sem);
					sem_wait(&empty);
					sem_wait(&sem);
    			}
    			if (buff[thread_index-1].itemid==0 && over) {
      				
      				sem_post(&empty);	
				sem_post(&sem);
       				return NULL;  
    			}
    			sem_post(&sem);
			
			for (int i=0; i<bufsize ; i++){
				char topic[16];
				char val[3];
				char newuid[5];
				char temparr[30];
				int temparr_new=0;
				//shared variable
				sem_wait(&sem);
				
				if(buff[thread_index-1].itemid!=0){
					strcpy(temparr, buff[thread_index-1].tuple_ptr[i]);
					buff[thread_index-1].itemid--;
					numbercons++;
					
					free(buff[thread_index-1].tuple_ptr[i]);
					buff[thread_index-1].tuple_ptr[i]=malloc(28*sizeof(char));
					strcpy(newuid, buff[thread_index-1].id);
					temparr_new=1;
					sem_post(&empty);
					sem_post(&full);
				}
				
				sem_post(&sem);
				//shared variable
				if(temparr_new){
					for(int i=0; i<=(strlen(temparr)); i++)
					{
						if(i>=6 && i<=20)
						{
							topic[i-6]=temparr[i];	
						}

						if(strlookup(temparr))
						{	
							if(i>=22 && i<=24)
							{
								val[i-22]=temparr[i];	
							}
						}
						else
						{
							if(i>=22 && i<=23)
							{
								val[i-22]=temparr[i];	
							}
						}			
					}
					if(!strlookup(temparr)){
						val[2]='\0';
					}
					strcpy(outbuffer[thread_index-1].usrid, newuid);
					for(int y=0; y<1000; y++){
						if(strcmp(outbuffer[thread_index-1].tuple[y],topic)==0){
							outbuffer[thread_index-1].score[y]+=atoi(val);
							break;
						}
						else if(outbuffer[thread_index-1].tuple[y][0]=='\0'){
							strcpy(outbuffer[thread_index-1].tuple[y],topic);
							outbuffer[thread_index-1].top++;
							outbuffer[thread_index-1].score[y]=atoi(val);
							break;
						}
					}


				}

			}
		
    }
    return NULL;
}

int main(int argc, char *argv[])
{
        printf("hello");
	bufsize=atoi(argv[2]);
	if(atoi(argv[1])==0)
		bufsize=atoi(argv[1]);
	numred=atoi(argv[2]);
	pthread_t threads[numred];
	void *status;
	pthread_attr_t attr;
	sem_init(&sem, 0, 1);
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, 0);
     	printf("hello");
	if(bufsize<5){
		bufsize=5;}
	for(int i=0; i<numred; i++){
		buff[i].tuple_ptr = malloc(bufsize*sizeof(char *));
		buff[i].itemid=0;
	}
	for(int i=0; i<numred; i++){
		for(int j=0; j<bufsize; j++){
			buff[i].tuple_ptr[j] = malloc(28*sizeof(char));
		}
	}
	for(int i=0; i<numred; i++){
    	outbuffer[i].tuple = malloc(3000*sizeof(char *));
    }
    for(int i=0; i<numred; i++){
		for(int j=0; j<1000; j++){
			outbuffer[i].tuple[j] = malloc(16*sizeof(char));
		}
	}
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	for(int i=0;i<=numred;i++)
  	{
    	if (i < 1)
       		pthread_create(&threads[i], &attr, mapper_thread, (void *)i); 
    	else pthread_create(&threads[i], &attr, reducer_thread, (void *)i);; 
   }

   /* Wait on the other threads */

	for(int i=0;i<=numred;i++) {
  		pthread_join(threads[i], &status);
  		
  	}

	

	for(int i=0; i<numred; i++){
		for(int j=0; j<outbuffer[i].top; j++){
		printf("(%s,%s,%d) \n", outbuffer[i].usrid,outbuffer[i].tuple[j],outbuffer[i].score[j]);	
		}
	}
	
    return 0;
 
	
	pthread_attr_destroy(&attr);
    pthread_exit(NULL);
}



