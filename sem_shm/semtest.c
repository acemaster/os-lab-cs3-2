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
	key_t key=1234;
	int pid;
	int *shm;
	int retstatus;
	int sem_id;
	int *arr=(int *)malloc(3);
	sem_id=sem_init(3,arr);
        printf("%d",sem_status(sem_id,0)); 
        sem_change(sem_id,0,1);
        printf("%d",sem_status(sem_id,0)); 
	exit(0);
}
   
          
 
