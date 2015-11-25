#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

int ID;
union semun
{
  int val;
  struct semid_ds *buf;
  unsigned short int *array;
};



int state=1;
int sem_init(int key,int n, int *array)
{
   key_t h=key;
   int sem_id=semget(h,n,IPC_CREAT|0666);
   if(sem_id ==-1){
      printf("Semaphore problem\n");
      exit(0);
  }
  union semun temp;
  temp.array=(unsigned short int*)array;
  semctl(sem_id,0,SETALL,temp);
  return sem_id;
}



void sem_change(int sem_id, int sem_no,int amount)
{
  struct sembuf temp;
  temp.sem_num=sem_no;
  temp.sem_flg=0;
  temp.sem_op=amount;
  if(semop(sem_id,&temp,1) == -1)
   {
      printf("Semop problem");
      exit(0);
   }
}



int sem_status(int sem_id, int sem_no){
        int val=semctl(sem_id,sem_no,GETVAL);
        return val;
}


void sem_destroy(int sem_id){
     int val=semctl(sem_id,0,IPC_RMID);
}
