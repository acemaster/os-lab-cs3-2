#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "semfunc.c"


/* This is the producer consumer problem using shared memory segments
 * The sem_init sem_change functions are defined in the semfunc.c
*/


int con_flag=0;
int prod_flag=0;
int max_count =10;
int count=0;
char value='a';
int main(){
	int prod_count=0;
	int con_count = 0;
	int shmid;
	key_t key=1234;
	int pid;
	int *shm;
	int retstatus;
	int sem_id;
	int *arr=(int *)malloc(3);
	sem_id=sem_init(3,arr);
	printf("Sem id: %d\n",sem_id);
	shm=(int *)malloc(50);
	if((shmid = shmget(key,27,IPC_CREAT|0666))<0)
	{
   		printf("SHM error...........\n");
   		exit(0);
	}

	if((shm=(int *)shmat(shmid,NULL,0)) == (char *)-1){
       		printf("Attachment error\n");
       		exit(0);
	}

	pid=fork();

	if(pid == 0)
	{
   		printf("Producer is going on:\n");
   		while(count < max_count)
   		{
          		sem_change(sem_id,0,1);
           		count++;
          		sem_change(sem_id,1,1);
           		*(shm+prod_count)=value;
            		printf("Producer produced : %d\n",value);
            		value++;
           		prod_count++;
                        if(count == 1)
                           sem_change(sem_id,2,1);
                        sem_change(sem_id,1,-1);
         		sem_change(sem_id,0,-1);
          		sleep(1);

    		}
	}
	else{
               int pid2=fork();
                if(pid2 == 0){
 			printf("Consumer is starting");
 			while(con_count < max_count){
   				sem_change(sem_id,0,1);
                                count--;
   				sem_change(sem_id,2,1);
   				printf("Value consumed: %d\n",*(shm+con_count));
   				con_count++;
                                if(count < max_count)
                                    sem_change(sem_id,1,1);
                                sem_change(sem_id,2,-1);
   				sem_change(sem_id,0,-1);
   				sleep(3);
			}
                }
                else{
			pid=wait(retstatus);
                }
	}
	exit(0);
}
   
          
 
