#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "semfunc.c"


/* This is the producer consumer problem using shared memory segments
 * The sem_init sem_change functions are defined in the semfunc.c
*/

int reader_count=2;
int writer_count=2;
int pid;
int reader_index[2]={0};
int writer_index=0;
int waiting_writers=0;
int waiting_readers=0;
char ch='a';
void reader(int i, int semid,int *shm){
 	while(1){
        waiting_readers++;
	semchange(semid,0,1);
         semchange(semid,1,1);
        printf("\tReader %d read %c\n",i,*(shm+reader_index[i]));
        reader_index[i]++;
        if(waiting_writers>0)
             {
                semchange(semid,1,-1);
              }
        semchange(semid,0,-1);
       }

}


void writer(int i,int semid, int shm){
     while(1){
      waiting_writers++;
      semchange(semid,1,1);
      semchange(semid,0,1);
       semchange(semid,2,1);
           *(shm+writer_index)=ch;
           ch++;
           writer_index++;
            printf("Writer %d wrote %c\n",i,ch-1);
       semchange(semid,2,-1);
      semchange(semid,0,-1);
      if(waiting_readers>0)
           semchange(semid,1,-1);
     }
}
              
int main(){
	int shmid;
	key_t key=1234;
	int pid;
	int *shm;
	int retstatus;
	int semid;
	int *arr=(int *)malloc(3);
	semid=sem_init(5656,3,arr);
	printf("Sem id: %d\n",semid);
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
        int pid2=fork();
        if(pid2 ==0)
       {
        	int i;
        	for(i=0;i<reader_count;i++)
		{
			pid=fork();
                	if(pid == 0)
                 	{
                    	reader(i,semid,shm);
                    	break;
                 	}
        	}
       }
       else{
       		int i;
                for(i=0;i<writer_count;i++)
                {
                       pid=fork(); 
                       if(pid == 0)
                       { 
                          writer(i,semid,shm); 
                          break;
                       }
                } 
         
		pid=wait(retstatus);
        	sem_destroy(semid);
		exit(0);
      }
}
   
          
 
