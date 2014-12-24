#include <malloc.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include "mythread.h"

ucontext_t *initContext;

struct queue;
volatile int totalcount=0,count=0;
volatile int flag=0;
volatile int ide=1000;
//states running =1 ready=2 blocked=3 terminated =4
ucontext_t cont1;
typedef struct Thread{
	ucontext_t mycontext;
	int id; //for identification of thread
	int join; //join=0 (means not waiitng for any thread) join=1 (means waiting for 1 thread) join=2 (means waiting for all children,joinAll)
	int numChildWaiting; //(indicates the number of active children. Will be zero by default. Will be incremented as childs are created
	//struct Thread *next;
	struct Thread *parent;
	struct Thread *forChild;
	struct queue *childrenQ;

}Thread;

typedef struct listNode{
	Thread *qthread;
	struct listNode *next;
}listNode;

typedef struct _list{
	listNode *first;
	listNode *last;
}_list;

typedef struct semaphore{
	int value;
	_list *blockedList;
}semaphore;

typedef struct node{
	Thread *qthread;
	struct node *next;
}node;

typedef struct queue{
	node *first;
	node *last;
}queue;

queue *readyQueue, *blockedQueue;


_list *createList(void)
{
	_list *semlist;
	semlist=(_list *)calloc(1,sizeof(_list));
	semlist->first=NULL;
	semlist->last=NULL;
	return semlist;
}

void insertInList(_list *l,Thread *t)
{
	listNode *n;
	n=(listNode *)calloc(1,sizeof(listNode));
	n->qthread=t;
	n->next=NULL;
	//printf("Insert happened here \n");
	if(l->first==NULL)
	{
		
		l->first=n;
		l->last=n;		
	}
	else
	{
		l->last->next=n;
		l->last=n;
	}

	return;
}

Thread *deList(_list *q)
{
	if(q->first == NULL  )
	{
		printf("list is empty");
		//setcontext(&(runningT->mycontext));
	}
	else if(q->first==q->last )
	{
		
		listNode *buff;
		buff= q->first;
		q->first=NULL;
		q->last=NULL;
		return (buff->qthread);
	}
	else
	{
		
		listNode *buff;
		buff= q->first;
		q->first = q->first->next;
		return (buff->qthread);
	}
		
}





queue* createQueue(){
	queue *tqueue;
	tqueue = (queue *)calloc(1,sizeof(queue));
	tqueue->first= NULL;
	tqueue->last=NULL;
	return tqueue;
}

Thread *runningT=NULL,*buffer=NULL,*initT=NULL; 
insertInQueue(queue *q, Thread *t){

	node *n;
	n=(node *)calloc(1,sizeof(node));
	n->qthread=t;
	n->next=NULL;
	
	if(q->first==NULL)
	{
		
		q->first=n;
		q->last=n;		
	}
	else
	{
		q->last->next=n;
		q->last=n;
	}

	return;
}

Thread* deQueue(queue *q)
{
	if(q->first == NULL  )
	{
		//printf("queue is empty for %d \n",runningT->id);
		return NULL;
		
	}
	else if(q->first==q->last )
	{
		
		node *buff;
		buff= q->first;
		q->first=NULL;
		q->last=NULL;
		return (buff->qthread);
	}
	else
	{
		
		node *buff;
		buff= q->first;
		q->first = q->first->next;
		return (buff->qthread);
	}
	
}

/***********************************************************/
int  deQueueChildren(queue *q,Thread *t)
{
	
	
	//printf("In deque child queue \n");
	node *temp,*behind;
	temp=q->first;
	behind=temp;
	if(temp==NULL)
		return 0;


	while(temp!=NULL)
	{
		if (temp->qthread== t)
		{
			if (q->first==q->last)
			{
				q->last= NULL;
				q->first=NULL;
				//printf("Dequeued the last child \n");
				return 1;
			}
			else 
			{
				if(behind!=temp)
				behind->next=temp->next;
				else
				{
					q->first=temp->next;
				}	
				//printf("first child is %d \n",q->first->qthread->id);
				//printf("Dequeued a child \n");
				return 1;
			}
		}
	  behind=temp;
	  temp=temp->next;
	}
	

		
}


/* ******************************************* */

int  deQueueParentBlocked(queue *q,Thread *t,Thread *p)
{
	
	if(q==NULL)
	return 0;
	
	node *temp,*behind;
	temp=q->first;
	behind=temp;
	if(temp==NULL)
		return 0;

	while(temp!=NULL)
	{
		if (temp->qthread== t)
		{
			if (q->first==q->last)
			{
				q->last= NULL;
				q->first=NULL;
				//printf("Dequeued the last child \n");
				insertInQueue(readyQueue,(temp->qthread));
				return 1;
			}
			else 
			{
				if(behind!=temp)
				behind->next=temp->next;
				else
				{
					q->first=temp->next;
				}	
				//printf("first child is %d \n",q->first->qthread->id);
				//printf("Dequeued a child \n");
				insertInQueue(readyQueue,(temp->qthread));
				return 1;
			}
		}
	  behind=temp;
	  temp=temp->next;
	}
	
}

int sizeOfQueue(queue *q)
{
	node *f,*l,*p;
	f=q->first;
	l=q->last;
	if (f==NULL && l== NULL)
		return 0;
	int num=0;
	p=f;
	while(p!=NULL)
	{
		num++;
		p=p->next;
	}
	return num;
}

/* ******************************************* */


void MyThreadInit(void(*start_funct)(void *), void *args)
{
	//printf("entered My thread init \n");
	if(flag==1)
	{
		return; //should return as init is already called by the function
	}
	flag=1;
	//++totalcount;
	initContext= (ucontext_t *)calloc(1,sizeof(ucontext_t));
	
	readyQueue = createQueue();	
	blockedQueue = createQueue();
	
	Thread *_mainThread;
	_mainThread =(Thread *) calloc(1,sizeof (Thread));
	//creating a context
	getcontext( &(_mainThread->mycontext) );
        _mainThread->mycontext.uc_link=NULL;
	_mainThread->mycontext.uc_stack.ss_sp= calloc(1024*16,sizeof (char));
	_mainThread->mycontext.uc_stack.ss_size=1024*16;
	_mainThread->parent=_mainThread;
	_mainThread->numChildWaiting=0;
	_mainThread->join=2;
	_mainThread->id=ide;
	_mainThread->childrenQ=createQueue();
	++ide;
	
	_mainThread->forChild=NULL;
	
	//
	makecontext(&(_mainThread->mycontext), (void(*)(void))start_funct,1,args);
	initT=_mainThread;
	//printf("The main thread from init is %d",initT->id);
	runningT=_mainThread;
	//insertInQueue(readyQueue, _mainThread) ;
	//printf("The main thread from main thread is  is %d",_mainThread->id);
	swapcontext(initContext, &(_mainThread->mycontext));
	runningT=initT;
	//printf("about to call join all of the init thread \n");
	MyThreadJoinAll();
	//printf("Last step of the main thread with id %d",_mainThread->id);
	return;
}


/******************************************/
MyThread MyThreadCreate(void(*start_funct)(void *), void *args)
{
	//printf("Entered the create thread function \n");
	Thread *_newThread;
	_newThread =(Thread *)calloc(1,sizeof (Thread));
	
	getcontext( &(_newThread->mycontext) );

	_newThread->mycontext.uc_stack.ss_sp= calloc(1024*16,sizeof (char));
	_newThread->mycontext.uc_stack.ss_size=1024*16;
	
	_newThread->parent=runningT;
	_newThread->numChildWaiting=0;
	_newThread->join=0;
	_newThread->id=ide;
	_newThread->forChild=NULL;
	_newThread->childrenQ=createQueue();
	insertInQueue(runningT->childrenQ,_newThread);
	++ide;

	_newThread->forChild=NULL;
	++(runningT->numChildWaiting);

	makecontext(&(_newThread->mycontext), (void(*)(void))start_funct,1,args);
	insertInQueue(readyQueue, _newThread  ) ;
	 
	return (void *)_newThread; 
	
}


/* ******************************************* */


void MyThreadYield(void)
{
	if( readyQueue->first==NULL)
	{	
		//as no one is in the waiting queue. This thread can continue running.
		return;
	}
	else 
	{
		
		buffer=runningT;
		runningT=deQueue(readyQueue);		
		insertInQueue(readyQueue,buffer);
			
		swapcontext(&(buffer->mycontext),&(runningT->mycontext));
		
		return;
		
	}
}
/* ******************************************* */
int checkChildren(queue *q, Thread *t)
{
	if ( (q==NULL) || (t==NULL) )
	return 0;
	node *n;
	n=q->first;
	while(n!=NULL)
	{
		if((n->qthread)==t)
			return 1;
		n=n->next;
	}
}

/* ******************************************* */

int MyThreadJoin(void * childT)
{
	Thread *child= (Thread *)childT;
	if ( child->parent != runningT)
	{
		//printf("The thread is not an immeadiate child of the calling thread \n");
		return -1;
	}
	else if (runningT->numChildWaiting==0) //indicates child thread is already terminated thread
	{
		return 0;
	}

	else if(childT ==NULL) {
		return 0 ; //waiting on a NULL child is equivalent to continuing execution
	}
	else if( checkChildren(runningT->childrenQ,child)) 
	{
		runningT->join=1;
		runningT->forChild=child;
		insertInQueue(blockedQueue,runningT);
		buffer=runningT;
		runningT=deQueue(readyQueue);
		swapcontext(&(buffer->mycontext),&(runningT->mycontext));
		return 0;
	}	
}

/* ******************************************* */

void MyThreadJoinAll(void)
{
	
	/*if(runningT==initT)
	{
		if(initT->childrenQ->first==NULL)
			return;
		else 
		{
			runningT->join=2;
			buffer=runningT;
			insertInQueue(blockedQueue,buffer);
			runningT=deQueue(readyQueue);
			swapcontext(&(buffer->mycontext),&(runningT->mycontext));
			return;
		}
	}*/
	if (runningT->childrenQ->first==NULL) //indicates child thread is already terminated thread
	{
		return;
	}
	else 
	{
		runningT->join=2;
		buffer=runningT;
		insertInQueue(blockedQueue,buffer);
		runningT=deQueue(readyQueue);
		swapcontext(&(buffer->mycontext),&(runningT->mycontext));
		return;
	}
}

/* ******************************************* */

void MyThreadExit(void)
{
	int decision,test;
	buffer=runningT;
	
	/*if( runningT==initT)
	{
		//runningT=deQueue(readyQueue);
		//printf("the new runnint thread is %d \n",runningT->id);
		//printf("shit case1 for %d \n",runningT->id);
		setcontext(initContext);
		return;		
	}*/	
	
	if(runningT->parent->join ==0)
	{
		
		test=deQueueChildren((runningT->parent->childrenQ),runningT);
		--(runningT->parent->numChildWaiting);	
		runningT=deQueue(readyQueue);
		if(runningT==NULL)
		{
			free(buffer);
			setcontext(initContext); 
		}	
		//printf("the new running thread is %d \n",runningT->id);
		free(buffer);
		setcontext(&(runningT->mycontext));
	}
	else if (runningT->parent->join == 1)
	{	
				
		test=deQueueChildren((runningT->parent->childrenQ),runningT); //removes the child from parents children list
		if(runningT->parent->forChild==runningT)
		{		
			deQueueParentBlocked(blockedQueue,runningT->parent,runningT);
			runningT->parent->join=0;
			runningT->parent->forChild=NULL;
		}
		
		--(runningT->parent->numChildWaiting);
			
		runningT=deQueue(readyQueue);
		if(runningT==NULL)
		{
			free(buffer);
			setcontext(initContext); 
		}	

		free(buffer);
		setcontext(&(runningT->mycontext));			
	}
	else if ( runningT->parent->join ==2)
	{
		
		test=deQueueChildren((runningT->parent->childrenQ),runningT);		
		--(runningT->parent->numChildWaiting);
		if(runningT->parent->childrenQ->first==NULL)
		{
			//printf("the children queue is empty \n");
			runningT->parent->join=0;
			deQueueParentBlocked(blockedQueue,runningT->parent,runningT);
		}	
		runningT=deQueue(readyQueue);
		if(runningT==NULL)
		{
			//free(buffer);
			setcontext(initContext); 
		}	
		//printf("the new running thread is %d \n",runningT->id);
		free(buffer);
		setcontext(&(runningT->mycontext));
	}
}


/* ******************************************* */

/* ******************************************* */

MySemaphore MySemaphoreInit(int initialValue)
{
	semaphore *_sem=(semaphore*)calloc(1,sizeof(semaphore));
	_sem->value=initialValue;
	_sem->blockedList=createList();
	
	return (MySemaphore)_sem;
}
/* ******************************************* */
void MySemaphoreSignal(MySemaphore sem)
{
	if(sem==NULL)
	{
		printf("Semaphore passed is NULL \n");
		return;
	}
	semaphore *_sem=NULL;
	_sem=(semaphore*)sem;
	(_sem->value)++;
	if(_sem->value<=0)
	{
		
		buffer= deList(_sem->blockedList);
		insertInQueue(readyQueue,buffer);
	}
}
/* ******************************************* */

int MySemaphoreDestroy(MySemaphore sem)
{
	semaphore *_sem = NULL;
	_sem=(semaphore *)sem;
	if(_sem==NULL)
	return 0;
	else if ((_sem->blockedList)!=NULL)
	{
		return -1;
	} 
	else if ( (_sem->blockedList)==NULL)
	{
		free(_sem->blockedList);
		free(_sem);
		return 0;
	}
}
/* ******************************************* */

void MySemaphoreWait(MySemaphore sem)
{
	Thread *temp1;	
	semaphore *_sem=NULL;
	_sem=(semaphore *)sem;
	--(_sem->value);
	if((_sem->value)<0)
	{
		insertInList(_sem->blockedList,runningT);
		temp1=runningT;
		runningT=deQueue(readyQueue);
		swapcontext(&(temp1->mycontext),&(runningT->mycontext));
	}
}


/**********************************************/


