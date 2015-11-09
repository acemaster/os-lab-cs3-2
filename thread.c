#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#define MAX_THREAD 20
#define MEM 64000
struct sigset_t act1;
int id,count;
ucontext_t Main;
struct itimerval it;
ucontext_t last;
struct sthread_t{
	int s_id;
	int s_size;
	ucontext_t * thread;
};
void kill_thread();
void sthread_init(struct sthread_t  * new_task){
		new_task->thread=(ucontext_t *)malloc(sizeof(ucontext_t));
		getcontext(new_task->thread);
		getcontext(&last);
		last.uc_stack.ss_sp=malloc(MEM);
		last.uc_stack.ss_size=MEM;
		last.uc_stack.ss_flags=0;
		makecontext(&last,kill_thread,0);
		new_task->thread->uc_link=&last;
		new_task->thread->uc_stack.ss_sp=malloc(MEM);
		new_task->thread->uc_stack.ss_size=MEM;
		new_task->thread->uc_stack.ss_flags=0;
		new_task->s_size=MEM;
}
struct queue{
	struct sthread_t * task;
	struct queue * next;
};
void init(struct itimerval it);
void schedule();
struct queue * head,*tail;
struct sthread_t * currrent_thread;
void block()
{
	sigfillset(&act1);
	sigprocmask(SIG_BLOCK,&act1,NULL); //block all signals
}
void unblock()
{
	sigprocmask(SIG_UNBLOCK,&act1,NULL);   //restore blocked signals
	sigemptyset(&act1);
}
int add_to_queue(struct sthread_t * new_task)
{
	printf("adding to queue\n");
	block();
		if(head==NULL)  //this is the first thread
		{
			getcontext(&Main);
			init(it);
			printf("done timer part\n");
			head=(struct queue *)malloc(sizeof(struct queue) );
			tail=head;
			head->next=NULL;
			tail->next=NULL;
			head->task=new_task;
			new_task->s_id=id;
			id++;
			count++;
			unblock();
			printf("changing context\n");
			return 1;
		}
		else
		{
			if(count==MAX_THREAD)
			{
				unblock();
				return -1;
			}
			else
			{
				struct queue * temp=(struct queue *)malloc(sizeof(struct queue));
				temp->next=NULL;
				temp->task=new_task;
				new_task->s_id=id;
				id++;
				count++;
				tail->next=temp;
				tail=temp;
				unblock();
				return 0;
			}
		}
}
void kill_thread()
{
	ucontext_t temp;
	if(head->next==NULL)
	{
		currrent_thread=NULL;
		it.it_interval.tv_sec=0;
		it.it_interval.tv_usec=0;
		it.it_value.tv_sec=0;
		it.it_value.tv_usec=0;
		setitimer(ITIMER_PROF,&it,NULL);
		setcontext(&Main);
		return ;
	}
	else
	{
		currrent_thread=head->next->task;
		head=head->next;
		swapcontext(&temp,currrent_thread);
	}
}
int sthread_create(struct sthread_t * new,void (* fn)(),void * arg)
{
	printf("Receive rquest\n");
	sthread_init(new);
	printf("init done\n");
	makecontext(new->thread,fn,1,arg);
	printf("new context done");
	int ret=add_to_queue(new);
	return ret;
}
void schedule(int l)
{
	printf("scehduling thread\n");
	  if(currrent_thread==NULL)    //no thread executing till now apart from main thread
	{
		currrent_thread=head->task;
		printf("swapping \n");
		swapcontext(&Main,currrent_thread->thread);
	}
	else
	{
		if(head->next==NULL) //this is the last thread
		{
			it.it_interval.tv_sec=0;
	 		it.it_interval.tv_usec=0;
	 		it.it_value.tv_sec=0;
	 		it.it_value.tv_usec =0;
	 		setitimer(ITIMER_PROF, &it, NULL);
			setcontext(&Main);
		}
		else
		{
			currrent_thread=&head->next->task;
			add_to_queue(&head->task);
			swapcontext(&head->task,&(head=head->next)->task);
		}
	}
}
void init(struct itimerval it)
{
	 struct sigaction act, oact;
	 act.sa_handler=schedule;
	 sigemptyset(&act.sa_mask);
	 act.sa_flags = 0;
	 sigaction(SIGPROF, &act, &oact);
	 it.it_interval.tv_sec=0;
	 it.it_interval.tv_usec=1000;
	 it.it_value.tv_sec=0;
	 it.it_value.tv_usec =5000;
	 setitimer(ITIMER_PROF, &it, NULL);
}
void fnc(void)
{
    while(1)
    {
        printf("ASDF\n");
    }
}
int main()
{
    struct sthread_t s;
    sthread_create(&s,(void*)(*fnc),NULL);
}
