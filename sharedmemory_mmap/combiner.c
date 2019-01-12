#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<malloc.h>
#include<string.h>
#include<memory.h>
#include<pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct node node;
struct node{
  node *next;
  int score;    
  char *string;
    char* uid;
};

void* retval;
void* mapper();
void* reducer(int);
void printdata (int listid);
void print_list (node *list);
node* update (char*id,char *newstring,int newscore,int listid);
//void printdata ();
//void print_list (node *list);
//void update (char*id,char *newstring,int newscore);

struct inp_tuple{
char* uid;
char action;
char* topic;
};
//data structure for output tuple
struct out_tuple{
//char* uid;
char uid[4];
char topic[15];
int score;
};
typedef struct inp_tuple inp_tuple;
typedef struct out_tuple out_tuple;
struct circular_buff
{
   out_tuple *buff;
   int head;
   int tail;
   int count;
   int size;
   char uid[4];
};
typedef struct circular_buff circbuff;
int addbuf(circbuff* ptr,out_tuple newdata);
int empty (circbuff* ptr);
int full (circbuff* ptr);
out_tuple rembuf(circbuff* ptr);
circbuff* new_circbuff(int size);
node **listptr;
circbuff **arrayptr;
int bufsize,numred;
int *done;
static pthread_mutex_t* listmtx;
pthread_mutex_t* mtxptr;
static pthread_cond_t* condempty[50];
static pthread_cond_t* condfull[50];



int main(int argc , char* argv[])
{
  bufsize = atoi(argv[1]);
//printf("Bufsize: %d",bufsize);
  numred  = atoi(argv[2]);
//printf("Numred : %d",numred);
  int ret;
  //int tmpcount=numred;
  //bufsize=5;
  done=mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *done =0;
  bufsize=1000;
  listmtx=(pthread_mutex_t*)mmap(NULL,sizeof(pthread_mutex_t),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1,0);
  mtxptr=(pthread_mutex_t*)mmap(NULL,sizeof(pthread_mutex_t),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1,0);
  //pthread_mutex_t mtx = *mtxinit;
  pthread_mutexattr_t attr;
  ret=pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(mtxptr,&attr);
  //printf("\n Ret = %d",ret);
  //static pthread_mutex_t listmtx = PTHREAD_MUTEX_INITIALIZER;
  
  pthread_condattr_t condattr;
  ret=pthread_condattr_setpshared(&condattr,PTHREAD_PROCESS_SHARED);
  //pthread_cond_t* condempty[numred];
  //pthread_cond_t* condfull[numred];
  for(int i=0;i<numred;i++)
   {
      condempty[i]=(pthread_cond_t*)mmap(NULL,sizeof(condempty[i]),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1,0);
      condfull[i]=(pthread_cond_t*)mmap(NULL,sizeof(condfull[i]),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1,0);

   }
   for(int i=0;i<numred;i++)
   {
       pthread_cond_init(condempty[i],&condattr);
       pthread_cond_init(condfull[i],&condattr);

   }
  //printf("\n Buffarray : %p",buffarray);
  //node* list_head_arr[numred];
  circbuff* buffarray[numred];
  node* list_head_arr[numred];
  //pid_t child[numred];
  int i=0;
  for(int i=0;i<numred;i++)
  { 
    //condempty[i]={PTHREAD_COND_INITIALIZER};
    //condfull[i]= PTHREAD_COND_INITIALIZER;

       buffarray[i]=new_circbuff(bufsize);
       list_head_arr[i]=NULL;
       //list_head_arr[i]=mmap(NULL,sizeof(node*), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);

       //child[i]=fork();
       //printf("\nBuffarray[%d] is %p", i,buffarray[i]);
       
       //printf("\nlist_head[%d] is %p", i,list_head_arr[i]);
  } 
  arrayptr = (circbuff**)mmap(NULL,sizeof(circbuff*), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  arrayptr = buffarray;
  //listptr= (node**)mmap(NULL,(sizeof(node*)*numred), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  listptr= list_head_arr;
  

  pid_t ret1=1;
  
  
  for(int j=0;j<numred;j++)
  {
      
      ret1=fork();
      
      if(ret1==0)
      {
        /* printf("\n list head is %p",listptr[j]);
        printf("\nIn child %d, unmapping unnecessary bufffers",j);
        printf("\n Value is %d",*done);
        for(int k=0;k<numred;k++)
        {
          if(k!=j)
          {
            //printf("In child %d, Unmapped : %p,status:%d",j,buffarray[k],munmap(buffarray[k],sizeof(circbuff)));
            munmap(arrayptr[k],sizeof(circbuff));
            munmap(condfull[k],sizeof(pthread_cond_t));
            munmap(condempty[k],sizeof(pthread_cond_t));
          }
        }
        
        //printf("\nGonna run reducers here in child %d",j);
         // pthread_exit(retval);*/
          reducer(j);
          printf("\nExit reducer %d",j);
          exit(0);
     }
    

   }
 
  if(ret1!=0)
  { 
    mapper();
   }
    

//printf("\nBefore wait");
    for(int i=0;i<numred;i++) // loop will run n times (n=5)
   {wait(NULL);}
  
  pthread_exit(retval);

  }     
      
   


void* mapper()
{
    //static int i=0;
    //i++;
    //printf("\nIn mapper");
    //, calling mapper %dth time",i);
    FILE *fh;
          fh = fopen("input.txt","r");
          if(fh==NULL)
         {
            perror("File open failed");
         }
         fseek(fh,0,SEEK_SET);
  char element[26]="\0";
  int count_ids=0;
  int frd;
  //read 25 characters from file each time until end of file
  while(frd=fread(element,25,1,fh))
 {   
    
    //printf("\nMapper:In current loop frd is %d",frd);
    
  fflush(stdin);
    
  //printf("\nMapper:element = %s\n", element);
  /*printf("\nBefore string compare");
  for(int k=0;k<numred;k++)
  {
      printf("\nuid at %d is %.4s", k, arrayptr[k]->uid);

  }


     //printf("\n new 1 check %s", uid_array[0]);
     //printf("\n new 2 check %s", uid_array[1]);
       //printf("\nNow in next while");

       */
     
     //extract data from tuple
  char *tuple = element;
  tuple++;
  tuple[strlen(tuple)-1]='\0';
  tuple[strlen(tuple)-2]='\0';

     //separate tokens
     char* token1 = strtok(tuple,",");
     
     char* token2 = strtok(NULL,",");
     
     char fin_token2[1];
     strcpy(fin_token2,token2);
     char* token3 = strtok(NULL,",");
     
    //initialise input tuple based on tokens 
    inp_tuple tup1 = {token1,fin_token2[0],token3} ;
    
    //calculate score based on action
    int score=0;
    switch(tup1.action)
    {
      case 'P' : score =50;
                 break;
      case 'L' : score =20;
                 break;
      case 'D' : score =-10;
                 break;
      case 'C' : score =30;
                 break;
      case 'S' : score =40;
                 
    } 
 //
  //initialize output tuple 
    out_tuple tup2 ;
   
   //tup2.uid = tup1.uid;
    strncpy(tup2.uid,tup1.uid,4);
    //printf("\ntup1.uid = %s", tup1.uid);
    strcat(tup2.uid,"\0");
    //printf("\n Tup2. uid right now %s",tup2.uid);
    strcpy(tup2.topic,tup1.topic);
     
    tup2.score = score;
    //printf("\nTup2 done\n");
    //print output tuple
    //printf("\n(%s)",tup2.uid);//tup2.topic,tup2.score); 
    //printf("\nAfter printing");

     int flag_uid_match = 0;
     //printf("\nBefore for loop");
       pthread_mutex_lock(mtxptr);
      

       
         //int uid_int = atoi(tup2.uid);
     for(int i=0;i<numred;i++)
    {
     
      if(!strncmp(arrayptr[i]->uid,tup2.uid,4))
            //if(uid_array[i]==uid_int)
      { 

          
          //printf("\nMapper:Matched at id %d, uid is %.4s \n",i,arrayptr[i]->uid);
          

          while(full(arrayptr[i]))
          {   
            //printf("\nMapper:Waiting for reducer");
            
            pthread_cond_wait(condfull[i],mtxptr);
          }
          //printf("\n Adding tuple with uid=%.4s,topic=%s,score=%d in buffer %d",tup2.uid,tup2.topic,tup2.score,i);
          addbuf(arrayptr[i],tup2); 
          pthread_cond_signal(condempty[i]); 
          flag_uid_match=1;
          break;

      }
       

    }
    
       if(!flag_uid_match)
    { 
      //printf("\nMapper:Adding in new buffer %d",count_ids);
      
      //uid_array[count_ids]=uid_int;
      
            strncpy(arrayptr[count_ids]->uid,tup2.uid,4);
            //strcat(uid_array[count_ids], "\0");
        //printf("\n Adding tuple with uid=%.4s,topic=%s,score=%d in buffer %d",tup2.uid,tup2.topic,tup2.score,count_ids);
      addbuf(arrayptr[count_ids],tup2); 
        pthread_cond_signal(condempty[count_ids]); 
      //printf("\nIn buffer %d uid is %s",count_ids,uid_array[count_ids]);
      count_ids++;
      //printf("\n Count ids is %d",count_ids);
     
      } 
        
      pthread_mutex_unlock(mtxptr);

   /*printf("\nAfter string compare");

    for(int i=0;i<numred;i++)
    {
     
      printf("\nuid at %d is %.4s",i,arrayptr[i]->uid);
  
      } 

    */
      

      //printf("\nMapper:Going to next while");
      
      
}


  pthread_mutex_lock(mtxptr);
  //printf("Setting done");
  *done=1;
   pthread_mutex_unlock(mtxptr); 

  for(int i=0;i<numred;i++)
  {
    pthread_cond_signal(condempty[i]);
  }


 
  //pthread_exit(retval);

}

void* reducer(int reducno)
{
    char element[26] ="";
  char *id = (char*)malloc(sizeof(char)*4);
    char *subject=malloc(16);
  int  givenscore;
  node* head=NULL;
        int ret;
    out_tuple tuple;
    //printf("\n Done is %d",*done);
    while(1)
    { //printf("\n In reducer %d",reducno);
                    pthread_mutex_lock(mtxptr);
                     if(*done && empty(arrayptr[reducno]))
            {  
              //printf("\n In condition,Done is %d",*done);
              //printf("\n Breaking now");
              pthread_mutex_unlock(mtxptr);
              break;
                  //printf("############%s\n", tuple.uid);
            }
            
                     while(empty(arrayptr[reducno]) && !*done)
                     {
                       
                       //printf("\nReducer%d:Waiting for mapper",reducno);
                       
                       pthread_cond_wait(condempty[reducno],mtxptr);
                     }
                     //while(!empty(arrayptr[reducno]) && !done)
                    
                        
                    //printf("Done is %d",done);
                    
                    tuple=rembuf(arrayptr[reducno]);
                   
                   //id = tuple.uid;
                   for(int i = 0; i<4; i++){ id[i] = tuple.uid[i];}
                   
                   //printf("\nReducer%d: Id:%.4s\n",reducno,id);
                   
                   subject = tuple.topic;
                   //printf("\nReducer%d:Subject:%.15s\n",reducno,tuple.topic);
                   givenscore=tuple.score;
                   //printf("\nReducer%d:Score:%d\n",reducno,tuple.score);
                   //printf("\n Adding to ll of %d",reducno);
                   //printf("\n Head before update in reducer %d is %p",reducno,listptr[reducno]);
                   listptr[reducno]=update(id,subject,givenscore,reducno);
                   //printf("\n Head after update in reducer %d is %p",reducno,listptr[reducno]);
                   pthread_cond_signal(condfull[reducno]);
                  
                   pthread_mutex_unlock(mtxptr);
                       
}
//printf("Out of while");
pthread_mutex_lock(listmtx);
 printdata(reducno); 
 pthread_mutex_unlock(listmtx);
 //printf("\nBack from printdata, exiting reducer %d\n",reducno);
 pthread_exit(retval);
        
      }


int empty (circbuff* ptr)
{

   
   if(((ptr->head)%(ptr->size) == ptr->tail) && (ptr->count<=0)) { return 1; }

   return 0;

}

int full(circbuff* ptr)
{
  if(((ptr->head)%(ptr->size) == ptr->tail) && (ptr->count)) { return 1; }
  return 0;
}

int addbuf(circbuff* ptr,out_tuple newdata)
{
  if(!full(ptr))
{
  //printf("\n In addbuf Adding tuple with uid=%.4s,topic=%s,score=%d in buffer with uid %.4s",newdata.uid,newdata.topic,newdata.score,ptr->uid);
  ptr->buff[ptr->head]=newdata;
  //ptr->buff[ptr->head].uid = (char*)malloc(sizeof(char)*4);
  //for(int j=0; j<4; j++) {
  //ptr->buff[ptr->tail].uid[j] = newdata.uid[j];}
  //printf("\n Adding tup##############le with uid=%.4s,topic=%s,score=%d in buffer with uid %.4s",ptr->buff[ptr->head].uid,ptr->buff[ptr->head].topic,ptr->buff[ptr->head].score,ptr->uid);

  //printf("\n Added element to location %d ", ptr->head);
  ptr->head= (ptr->head+1)%(ptr->size);
  ptr->count++;
  return 1;
}
  //printf("\n Buffer is full");
}
out_tuple rembuf(circbuff* ptr)
{

  //printf("\n In rembuf with uid %.4s",ptr->uid);
  if(!empty(ptr))
{
  out_tuple newdata;
  newdata = (ptr->buff[ptr->tail]);
  ptr->tail= (ptr->tail+1)%(ptr->size);
  ptr->count--;
  //printf("Sending tuple from buffer :");
  //printf("\n uid=%.4s,topic=%s,score=%d",newdata.uid,newdata.topic,newdata.score);
  return newdata;
} 
  //printf("\n Buffer empty");
}

circbuff* new_circbuff(int size)
{
  //circbuff* ptr = (circbuff*)malloc(sizeof(circbuff));
  circbuff *ptr =(circbuff*)mmap(NULL,sizeof(circbuff), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  memset(ptr,0,sizeof(circbuff));
  ptr->size= size;
  //out_tuple *buff = (out_tuple*)malloc(sizeof(*buff)*size);
  out_tuple *buff = (out_tuple*)mmap(NULL,(sizeof(*buff)*size), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  //printf("\nPointer is %p",ptr);
 // printf("Buffer ptr is %p",buff);
  ptr->buff=buff;
  return ptr;

}


node* update (char*id,char *newstring,int newscore,int listid)
{
  //
  //printf("\n In update");// listid is %d,uid is %s",listid,id);
  //
  
        // if list is empty, add new node in list
  pthread_mutex_lock(listmtx);
  if(listptr[listid]==NULL)
  {
    listptr[listid]=calloc(1,sizeof(node));
    //printf("\n Head in reducer is %p",listptr[listid]);
    listptr[listid]->string=calloc(1,strlen(newstring));
    strncpy(listptr[listid]->string,newstring,strlen(newstring));
                listptr[listid]->uid=calloc(1,strlen(id));
    strncpy(listptr[listid]->uid,id,strlen(id));
    
    listptr[listid]->next=NULL;
    listptr[listid]->score=newscore;
    
   
    
                
    //printf("\n Head in reducer is %p",listptr[listid]);

  }

        // if list is not empty and topic is same, generate new score
  else
  {   node *iter=listptr[listid];
    
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
                                strncpy(iter->next->uid,id,strlen(id));
        iter->next->string=calloc(1,strlen(newstring));
        strncpy(iter->next->string,newstring,strlen(newstring));
        iter->next->score=newscore;
                                iter->next->next=NULL;
                                
        break; 
      }

    } 
  }
  //printf("\n In LL%d now adding %.4s,%.15s,%d",listid,id,newstring,newscore);
  pthread_mutex_unlock(listmtx);
  return listptr[listid];
}

//print the linked list and free memory
void print_list (node *list)
{ if (list!=NULL)
  {       
               
    printf("(%.4s,%.15s,%d) \n",list->uid,list->string,list->score);
    
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

void printdata (int listid){
  //printf("In printdata");
  //printf("Entering printdata of %d",listid);
  print_list(listptr[listid]);
  free(listptr[listid]);
  listptr[listid]=0;
  //printf("Leaving printdata of %d",listid);
}


/*
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
  {   node *iter=list_head;
    
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
{ if (list!=NULL)
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
}*/