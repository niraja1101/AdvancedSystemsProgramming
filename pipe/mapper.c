#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
typedef struct inp_tuple inp_tuple;
typedef struct out_tuple out_tuple;

//data structure for input tuple
struct inp_tuple{
char* uid;
char action;
char* topic;
};
//data structure for output tuple
struct out_tuple{
char* uid;
char topic[15];
int score;
};


int main()
{ 
  //open input file
  FILE *fh;
  fh = fopen("input.txt","r");
  if(fh==NULL)
  {
    perror("File open failed");
  }

  //seek to beginning of file
  fseek(fh,0,SEEK_SET);
  char element[26]="";
  
  //read 25 characters from file each time until end of file
  while(fread(element,25,1,fh))
 { 
   
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
 
  //initialize output tuple 
  out_tuple tup2 ;
 
  tup2.uid = tup1.uid;
 
  strcpy(tup2.topic,tup1.topic);

  tup2.score = score;

  //print output tuple
  printf("(%s,%s,%d)\n",tup2.uid,tup2.topic,tup2.score); 
   
}
  printf("\n");
  
  return 0;

}
