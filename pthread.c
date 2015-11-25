#include<ucontext.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_THREAD 20
#define MEM 64000


struct sigaction actl;
int id,count;
ucontext_t Main,last;
struct itimerval it;
struct rthread_t{
	int r_id;
	int r_size;
	ucontext_t *thread;
};


void kill_thread();
void rthread_init(struct rthread_t *newtask)
{
	newtask->thread=(ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(newtask->thread);
	getcontext(&last);
	last.uc_stack.ss_sp=malloc(MEM);
	last.uc_stack.ss_size=MEM;
	last.uc_stack.ss_flags=0;
	makecontext(&last,kill_thread,0);
	newtask->thread->uc_link=&last;
	newtask->thread->uc_stack.ss_sp=malloc(MEM);
	newtask->thread->uc_stack.ss_size=MEM;
	newtask->thread->uc_stack.ss_flags=0;
	newtask->r_size=MEM;
}



struct 	queue{
	struct rthread_t *task;
	struct queue *next;
};


void init(struct itimerval it);
void schedule();
struct queue *head,*tail;
struct rthread_t *current_thread,*previous_thread;
void block(){
	sigfillset(&actl.sa_mask);
	sigprocmask(SIG_BLOCK,&actl.sa_mask,NULL);
}


void unblock(){
	sigprocmask(SIG_UNBLOCK, &actl.sa_mask,NULL);
	sigemptyset(&actl.sa_mask);
}


int add_to_queue(struct rthread_t *new_task){
	printf("ADDING TO QUEUE! \n");
	block();
	if(head == NULL){
		getcontext(&Main);
		init(it);
		head=(struct queue*)malloc(sizeof(struct queue));
		tail=head;
		head->next=NULL;
		tail->next=NULL;
		head->task=new_task;
		new_task->r_id=id;
		id++;
		count++;
		unblock();
		return 1;
	}
	else{
		if(count ==  MAX_THREAD){
			unblock();
			return -1;
		}
		else{
			struct queue *temp=(struct queue *)malloc(sizeof(struct queue));
			temp->next=NULL;
			temp->task=new_task;
			new_task->r_id=id;
			id++;
			count++;
			tail->next=temp;
			tail=temp;
			unblock();
			return 0;
		}
	}
}



void kill_thread(){
	ucontext_t temp;
	if(head->next == NULL){
		current_thread=NULL;
		it.it_interval.tv_sec=0;
		it.it_interval.tv_usec=0;
		it.it_value.tv_sec=0;
		it.it_value.tv_usec=0;
		setitimer(ITIMER_PROF,&it,NULL);
		setcontext(&Main);
		return;
	}else{
		current_thread=head->next->task;
		head=head->next;
		swapcontext(&temp,current_thread->thread);
	}
}



int rthread_create(struct rthread_t *newt, void (*fn)(), void *arg){
	printf("CREATING THREAD.........\n");
	rthread_init(newt);
	makecontext(newt->thread,fn,1,arg);
	int ret=add_to_queue(newt);
	printf("Created thread...........\n");
	return ret;
}



void schedule(){
	printf("Scheduling starts\n");
        int d;
        scanf("%d",&d);
	if(current_thread == NULL){
		current_thread=head->task;
		swapcontext(&Main,current_thread->thread);
	}
	else{
	printf("Checkling");
		if(head->next ==NULL){
			it.it_interval.tv_sec=0;
			it.it_interval.tv_usec=0;
			it.it_value.tv_sec=0;
			it.it_value.tv_usec=0;
			setitimer(ITIMER_PROF,&it,NULL);
			setcontext(&Main);
		}
		else{
			current_thread=head->next->task;
			add_to_queue(head->task);
                        previous_thread=head->task;
			head=head->next;
			swapcontext(previous_thread->thread,current_thread->thread);
		}
	}
	printf("Scheduling ends\n");
}


void init(struct itimerval it){
        printf("Initializing.........\n");
	struct sigaction act,oact;
	act.sa_handler=schedule;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	sigaction(SIGPROF,&act,&oact);
	it.it_interval.tv_sec=0;
	it.it_interval.tv_usec=100;
	it.it_value.tv_sec=0;
	it.it_value.tv_usec=5;
	setitimer(ITIMER_PROF,&it,NULL);
	printf("Completed initialization.......\n");
}

void testfunc(){
  int t=1;
  printf("hello...checking if this works u can sleep\n");
  while(t){
	printf("In funtion 1 %d",t);
	t++;
  }
}

void test2func(){
 int t=1;
  printf("\tThis is function 2 .....It should work\n");
  while(t){
	printf("\tIn funtion 2 %d",t);
	t++;
  }
}

int main(){
    struct rthread_t newthread;
    struct rthread_t newthread2;
    int ret=rthread_create(&newthread,testfunc,NULL);
    int ret2=rthread_create(&newthread2,test2func,NULL);
    while(1){};
    return 0;
}
			
		
	
		
		
		
