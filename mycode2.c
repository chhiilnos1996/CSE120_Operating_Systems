/* mycode2.c: your portion of the kernel
 *
 *   	Below are functions that are called by other parts of the kernel. 
 * 	Your ability to modify the kernel is via these functions.  You may
 *  	modify the bodies of these functions (and add code outside of them)
 *  	in any way you wish (however, you cannot change their interfaces).  
 */

#include "aux.h"
#include "sys.h"
#include "mycode2.h"
#include "stdio.h"
#include "limits.h"

#define TIMERINTERVAL 1	// in ticks (tick = 10 msec)
#define L 100000

/* 	A sample process table. You may change this any way you wish. 
 */

static struct {
	int n;
        long pass;	
	int valid;		// is this entry valid: 1 = yes, 0 = no
	int pid;		// process ID (as provided by kernel)
	
} proctab[MAXPROCS];

int num_0 = 0;
int sum = 0;
int front = 0;
int rear = -1;
int rr = 0;

/* 	InitSched() is called when the kernel starts up. First, set the
 * 	scheduling policy (see sys.h). Make sure you follow the rules
 *   	below on where and how to set it.  Next, initialize all your data
 * 	structures (such as the process table).  Finally, set the timer
 *  	to interrupt after a specified number of ticks. 
 */

void InitSched()
{
	int i;

	/* First, set the scheduling policy. You should only set it
	 * from within this conditional statement. While you are working
	 * on this assignment, GetSchedPolicy() will return NOSCHEDPOLICY. 
	 * Thus, the condition will be true and you may set the scheduling
	 * policy to whatever you choose (i.e., you may replace ARBITRARY).  
	 * After the assignment is over, during the testing phase, we will
	 * have GetSchedPolicy() return the policy we wish to test (and
	 * the policy WILL NOT CHANGE during the entirety of a test).  Thus
	 * the condition will be false and SetSchedPolicy(p) will not be
	 * called, thus leaving the policy to whatever we chose to test
	 * (and so it is important that you NOT put any critical code in
	 * the body of the conditional statement, as it will not execute when
	 * we test your program). 
	 */
	if (GetSchedPolicy() == NOSCHEDPOLICY) {	// leave as is
							// no other code here
		SetSchedPolicy(PROPORTIONAL);		// set policy here
							// no other code here
	}
		
	/* Initialize all your data structures here */
	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;
	}

	/* Set the timer last */
	SetTimer(TIMERINTERVAL);
}


/*  	StartingProc(p) is called by the kernel when the process
 * 	identified by PID p is starting. This allows you to record the
 * 	arrival of a new process in the process table, and allocate any
 * 	resources (if necessary). Returns 1 if successful, 0 otherwise. 
 */

int StartingProc(int p) 		
	// p: process that is starting
{
	int i;
	int min_pass;
	if(GetSchedPolicy()==LIFO||GetSchedPolicy()==FIFO){
		if(rear==MAXPROCS-1){
			rear = 0;
		}
		else{
			rear++;	
		}
		if(rear>=0){
			proctab[rear].valid=1;
			proctab[rear].pid=p;	
		}
		DoSched();
		return 1;
	} 
	else if(GetSchedPolicy()==PROPORTIONAL){
		min_pass=INT_MAX;
		for(i = 0; i < MAXPROCS; i++){
			//DPrintf("%d: v=%d p=%d\n",proctab[i].pid,proctab[i].valid,proctab[i].pass);
			if(proctab[i].valid==1&&proctab[i].pass<min_pass){
				//DPrintf("%d min pass\n",i);
				min_pass=proctab[i].pass;
			}
		}

		for(i = 0; i < MAXPROCS; i++) {
			if (!proctab[i].valid) {
				proctab[i].valid = 1;
				proctab[i].pid = p;
				proctab[i].n=0;
				num_0++;
				if(min_pass==INT_MAX)	proctab[i].pass=0;
				else	proctab[i].pass=min_pass;
				return 1;
			}
		}
	}
	else{
		for(i = 0; i < MAXPROCS; i++) {
			if (!proctab[i].valid) {
				proctab[i].valid = 1;
				proctab[i].pid = p;
				return 1;
			}
		}
	}	
	DPrintf("Error in StartingProc: no free table entries\n");
	return(0);
}
			

/*   	EndingProc(p) is called by the kernel when the process
 * 	identified by PID p is ending.  This allows you to update the
 *  	process table accordingly, and deallocate any resources (if
 *  	necessary).  Returns 1 if successful, 0 otherwise. 
 */


int EndingProc(int p)
	// p: process that is ending
{
	if(GetSchedPolicy()==FIFO){
		proctab[front].valid=0;
		if(front==MAXPROCS-1){
			front = 0;
		}
		else{
			front++;
		}
		return(1);
	}	
	else if(GetSchedPolicy()==LIFO){
		if(rear>=0) proctab[rear].valid=0;
		if(rear==0){
			rear = MAXPROCS-1;
		}
		else{
			rear--;
		}
		return(1);
	}	
	else if(GetSchedPolicy()==PROPORTIONAL){
		for(int i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid && proctab[i].pid == p) {
				proctab[i].valid = 0;
				sum -=proctab[i].n;
				if(proctab[i].n==0) num_0--;
				return(1);
			}
	
		}
	}
	else{
		for(int i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid && proctab[i].pid == p) {
				proctab[i].valid = 0;
				return(1);
			}
	
		}
	}
	DPrintf("Error in EndingProc: can't find process %d\n", p);
	return(0);
}


/* 	SchedProc() is called by kernel when it needs a decision for
 * 	which process to run next. It will call the kernel function
 * 	GetSchedPolicy() which will return the current scheduling policy
 *   	which was previously set via SetSchedPolicy(policy). SchedProc()
 * 	should return a process PID, or 0 if there are no processes to run. 
 */

int SchedProc()
{
	int min_pass;
	switch(GetSchedPolicy()) {
	
	case ARBITRARY:
		for(int i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				return(proctab[i].pid);
			}
		}
		break;

	case FIFO:
		/* your code here */
		return proctab[front].pid;	
		break;

	case LIFO:
		/* your code here */
		if(rear>=0) return proctab[rear].pid;
		else return 1;	
		break;

	case ROUNDROBIN:
		/* your code here */
		for(int i=rr;i<MAXPROCS;i++){
			if(proctab[i].valid){
				rr = (i+1)%MAXPROCS;
			 	return (proctab[i].pid);
			}
		}
		for(int i=0;i<rr;i++){
			if(proctab[i].valid){
				rr = (i+1)%MAXPROCS;
		 		return (proctab[i].pid);
			}
		}
		break;

	case PROPORTIONAL:
		/* your code here */
		min_pass = INT_MAX;
		int ret=-1;
		for(int i=0;i<MAXPROCS;i++){
			//if (proctab[i].valid>-1) DPrintf("%d:v=%d p= %d n=%d \n",proctab[i].pid,proctab[i].valid,proctab[i].pass,proctab[i].n);
			if((proctab[i].pass<=min_pass)&&(proctab[i].valid==1)){
				if(sum==100&&proctab[i].n==0) continue;
				min_pass=proctab[i].pass;
				ret = i;
			}
		}
		if(ret==-1) break;

		int stride=0;
		if(proctab[ret].n==0){
			stride =L*num_0/(100-sum);
		}
		else stride = L/proctab[ret].n;
		proctab[ret].pass+=stride;
		/*
		DPrintf("ret = %d stride = %d, pass = %d\n",ret,stride,proctab[ret].pass);
		DPrintf("\n");
		DPrintf("\n");
		*/
		return proctab[ret].pid;
		break;
	}
	
	return(0);
}


/*  	HandleTimerIntr() is called by the kernel whenever a timer
 *  	interrupt occurs.  Timer interrupts should occur on a fixed
 * 	periodic basis.
 */

void HandleTimerIntr()
{
	SetTimer(TIMERINTERVAL);

	switch(GetSchedPolicy()) {	// is policy preemptive?

		case ROUNDROBIN:		// ROUNDROBIN is preemptive
		       	DoSched();
			break;

		case PROPORTIONAL:		// PROPORTIONAL is preemptive
			DoSched();		// make scheduling decision
			break;

		default:			// if non-preemptive, do nothing
			break;
	}
}

/* 	MyRequestCPUrate(p, n) is called by the kernel whenever a process
 * 	identified by PID p calls RequestCPUrate(n).  This is a request for
 *   	n% of CPU time, i.e., requesting a CPU whose speed is effectively
 * 	n% of the actual CPU speed. Roughly n out of every 100 quantums
 *  	should be allocated to the calling process. n must be at least
 *  	0 and must be less than or equal to 100. MyRequestCPUrate(p, n)
 * 	should return 0 if successful, i.e., if such a request can be
 * 	satisfied, otherwise it should return -1, i.e., error (including
 * 	if n < 0 or n > 100). If MyRequestCPUrate(p, n) fails, it should
 *   	have no effect on the scheduling of this or any other process,
 * 	i.e., AS IF IT WERE NEVER CALLED.
 */

int MyRequestCPUrate(int p, int n)
	// p: process whose rate to change
	// n: percent of CPU time
{
	/* your code here */
	for(int i=0;i<MAXPROCS;i++){
		if(proctab[i].pid==p){
			if(n==0){
				if(proctab[i].n==0) return -1;
				num_0++;
				sum-=proctab[i].n;
				proctab[i].n=0;
				return 0;
			}
			else if(sum+n-proctab[i].n<=100){
				if(proctab[i].n==0) num_0--;
				sum=sum+n-proctab[i].n;
				proctab[i].n=n;
				return 0;
			}
			return -1;
		}		
	}
	return 0;
}
