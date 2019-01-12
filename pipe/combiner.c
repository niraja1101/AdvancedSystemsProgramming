#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/wait.h>

int main(int argc, char *argv[])
  {
     int fd[2],status;
     //create pipe
     pipe(fd);

     //spawn new process
     pid_t pid =fork();

     //run mapper in newly spawned process, connect write end of pipe to mapper output
     if(pid == 0) {
        
       close(fd[0]);
       dup2(fd[1],STDOUT_FILENO);
       
       execvp(argv[1],argv);
       printf("\n Fails to Execute");
       }

     //spawn another new process from parent and run reducer in this new child process
     else{
        close(fd[1]);
	
        pid_t pid1 =fork();
        if(pid1==0)
        {
           
           close(fd[1]);
           dup2(fd[0],STDIN_FILENO);
           execvp(argv[2],argv);
        }
        else{
        
	close(fd[0]);
       
	}
}
     
  
  return 0;   }
  
