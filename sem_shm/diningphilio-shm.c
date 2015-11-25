#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "semfunc.c"


/* This is the producer consumer problem using shared memory segments
 * The sem_init sem_change functions are defined in the semfunc.c
*/

int count=7;
int pid;
int phil[7]={0};
//0 for thuinking 1 for hungry 2 for done
void philosopher(int i,int semid,int semid2){	
   while(phil[i] != 2){
   //sem_change(semid2,0,1);
     if(sem_status(semid,i) == 0)
          {
              sem_change(semid,i,1);
              phil[i]=1;
              printf("Philosopher %d got 1 spoon\n",i);
              if(sem_status(semid,(i+1)%7)== 0){
                   sem_change(semid,(i+1)%7,1);
                   phil[i]=2;
                   printf("Philosopher %d got 2 spoons \n",i);
                   printf("Philosopher %d eating.......\n",i);
                   sem_change(semid,(i+1)%7,-1); 
                   printf("%d Released second spoon\n",i);
                   sem_change(semid,i,-1);
                   printf("%d Released first spoon\n",i);
                   //sem_change(semid2,0,-1);
                   printf("Philosopher %d finished eating......\n",i);
                   phil[i]=0;
                   usleep(rand()%30000000);
              }
              else{
                  printf("Philosopher %d didnt get other spoon...\n",i);
                  phil[i]=0;
                  sem_change(semid,i,-1);
         //         sem_change(semid2,0,-1);
                  usleep(rand()%30000000);
              }
           }
    else{
       printf("Philosopher %d didnt get any spoon\n ",i);
       //sem_change(semid2,0,-1);
       usleep(rand()%30000000);
    }
 }
}
int main(){
	int shmid;
	key_t key=1234;
	int pid;
	int *shm;
	int retstatus;
	int semid;
        int semid2;
	int *arr=(int *)malloc(count);
        int *arr2=(int *)malloc(1);
	semid=sem_init(5656,count,arr);
        semid2=sem_init(5657,1,arr2);
	printf("Sem id: %d\n",semid);
        int i;
        for(i=0;i<7;i++)
	{
		pid=fork();
                if(pid == 0)
                 {
                    philosopher(i,semid,semid2);
                    break;
                 }
        }
        
	pid=wait(retstatus);
        sem_destroy(semid);
        sem_destroy(semid2);
	exit(0);
}
   
          
 
