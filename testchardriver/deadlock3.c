#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/wait.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>

#define DEVICE "/dev/a5"
#define CDRV_IOC_MAGIC 'Z'
#define E2_IOCMODE1 _IOWR(CDRV_IOC_MAGIC, 1, int)
#define E2_IOCMODE2 _IOWR(CDRV_IOC_MAGIC, 2, int)

pthread_mutex_t lock;
int count=0;

void *threadfunc()
{
    int fd;
    fd = open(DEVICE, O_RDWR);

    if(fd == -1)
    {
	printf("File %s either does not exist or has been locked by another process\n", DEVICE);
	exit(-1);
    }
	
    pthread_mutex_lock(&lock);
    count++;
    pthread_mutex_unlock(&lock);

    while(count<2);

    close(fd);
    pthread_exit(NULL);

}

int main ()
{
    pthread_t threads[2];

    pthread_create(&threads[0], NULL, threadfunc, NULL);

    pthread_create(&threads[1], NULL, threadfunc, NULL);

    pthread_exit(NULL);
}

