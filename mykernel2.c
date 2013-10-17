/* mykernel2.c: your portion of the kernel (last modified 10/18/09)
 *
 *	Below are procedures that are called by other parts of the kernel.
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).
 * 
 */

#include "aux.h"
#include "sys.h"
#include "mykernel2.h"

#define TIMERINTERVAL 1	/* in ticks (tick = 10 msec) */

/*	A sample process table.  You may change this any way you wish.
 */
static int proc_counter = 0;
static int RR_counter = -1;
static double cpu = 1.00;
static struct {
	int valid;		/* is this entry valid: 1 = yes, 0 = no */
	int pid;		/* process id (as provided by kernel) */
    int order;
    double live_time;
    double exe_time;
    double utili;
    double request;
    int request_flag;
    double ratio;
} proctab[MAXPROCS];


/*	InitSched () is called when kernel starts up.  First, set the
 *	scheduling policy (see sys.h).  Make sure you follow the rules
 *	below on where and how to set it.  Next, initialize all your data
 *	structures (such as the process table).  Finally, set the timer
 *	to interrupt after a specified number of ticks.
 */

void InitSched ()
{
	int i;

	/* First, set the scheduling policy.  You should only set it
	 * from within this conditional statement.  While you are working
	 * on this assignment, GetSchedPolicy will return NOSCHEDPOLICY,
	 * and so the condition will be true and you may set the scheduling
	 * policy to whatever you choose (i.e., you may replace ARBITRARY).
	 * After the assignment is over, during the testing phase, we will
	 * have GetSchedPolicy return the policy we wish to test, and so
	 * the condition will be false and SetSchedPolicy will not be
	 * called, thus leaving the policy to whatever we chose to test.
	 */
	if (GetSchedPolicy () == NOSCHEDPOLICY) {	/* leave as is */
		SetSchedPolicy (PROPORTIONAL);		/* set policy here */
	}
		
	/* Initialize all your data structures here */
	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;

	}

	/* Set the timer last */
	SetTimer (TIMERINTERVAL);
}


/*	StartingProc (pid) is called by the kernel when the process
 *	identified by pid is starting.  This allows you to record the
 *	arrival of a new process in the process table, and allocate
 *	any resources (if necessary).  Returns 1 if successful, 0 otherwise.
 */

int StartingProc (pid)
	int pid;
{
	int i;
	for (i = 0; i < MAXPROCS; i++) {
		if (! proctab[i].valid) {
			proctab[i].valid = 1;
			proctab[i].pid = pid;
            proctab[i].order = proc_counter++;
            proctab[i].exe_time = 0.0;
            proctab[i].live_time = 0.0;
            proctab[i].utili = 0.0;
            proctab[i].request = 0.0;
            proctab[i].request_flag = 0;
            proctab[i].ratio = 0.0;
            unrequest_update();

                               //Set up a timestamp to record a process starting time
            if(GetSchedPolicy()==LIFO)
                DoSched();
			return (1);
		}
	}

	Printf ("Error in StartingProc: no free table entries\n");
	return (0);
}
			

/*	EndingProc (pid) is called by the kernel when the process
 *	identified by pid is ending.  This allows you to update the
 *	process table accordingly, and deallocate any resources (if
 *	necessary).  Returns 1 if successful, 0 otherwise.
 */


int EndingProc (pid)
	int pid;
{
	int i;

	for (i = 0; i < MAXPROCS; i++) {
		if (proctab[i].valid && proctab[i].pid == pid) {
			proctab[i].valid = 0;
            unrequest_update();
			return (1);
		}
	}

	Printf ("Error in EndingProc: can't find process %d\n", pid);
	return (0);
}


/*	SchedProc () is called by kernel when it needs a decision for
 *	which process to run next.  It calls the kernel function
 *	GetSchedPolicy () which will return the current scheduling policy
 *	which was previously set via SetSchedPolicy (policy). SchedProc ()
 *	should return a process id, or 0 if there are no processes to run.
 */

int SchedProc ()
{
	int i;
    int minOrder = 999999;
    int MinPid = -1;
    int count;
    double miniRatio = 999999.00;


    
	switch (GetSchedPolicy ()) {

	case ARBITRARY:

		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				return (proctab[i].pid);
			}
		}
		break;

	case FIFO:
            MinPid = 0;
        for (i = 0; i < MAXPROCS; i++) {
                
            if (proctab[i].valid && proctab[i].order < minOrder) {
                minOrder = proctab[i].order;
                MinPid = proctab[i].pid;
            }
        }
            if (MinPid) {
                return MinPid;
            }
      		/* your code here */
            

		break;

	case LIFO:

            for(i = MAXPROCS - 1; i >= 0; i--)
            {
                if(proctab[i].valid)
                    return(proctab[i].pid);
            }
            break;		/* your code here */


	case ROUNDROBIN:
            //Printf("Start Schedualing");
            count = 0;
            i = RR_counter;
            while(1)
            {
                i = (i + 1) % MAXPROCS;
                if(proctab[i].valid)
                {
                    RR_counter = i;
                    return (proctab[i].pid);
                }
                count++;
                if(count == MAXPROCS)
                    return 0;
            }
            break;


	case PROPORTIONAL:
            //Printf("proportional");
            for (i = 0; i < MAXPROCS; i++) {
               // Printf("1");
                if (proctab[i].valid) {
                    //Printf("2");
                    proctab[i].live_time++;
                    //Printf("3");
                    proctab[i].utili = proctab[i].exe_time / proctab[i].live_time;
                   // Printf("4");
                    proctab[i].ratio = proctab[i].utili / proctab[i].request;
//                    Printf("%f",proctab[i].ratio);
//                    Printf("%f",proctab[i].exe_time);
//                    Printf("%f",proctab[i].live_time);
//                    Printf("%f",proctab[i].utili);
//                    Printf("%f",proctab[i].request);
                    if (proctab[i].ratio < miniRatio) {
                     //   Printf("6");
                        miniRatio = proctab[i].ratio;
                        MinPid = i;
//                        Printf("%f", miniRatio);
//                        Printf("%d", MinPid);
                    }
                }
            }
            if (MinPid == -1) {
                //Printf("4");
                return 0;
            }
            proctab[MinPid].exe_time++;
            return proctab[MinPid].pid;

		break;

	}
	
	return (0);
}


/*	HandleTimerIntr () is called by the kernel whenever a timer
 *	interrupt occurs.
 */

void HandleTimerIntr ()
{
	SetTimer (TIMERINTERVAL);
    //Printf("in Hadel TimerIntr");

	switch (GetSchedPolicy ()) {	/* is policy preemptive? */

	case ROUNDROBIN:/* ROUNDROBIN is preemptive */
	case PROPORTIONAL:		/* PROPORTIONAL is preemptive */
		DoSched ();		/* make scheduling decision */
		break;

	default:			/* if non-preemptive, do nothing */
		break;
	}
}

/*	MyRequestCPUrate (pid, m, n) is called by the kernel whenever a process
 *	identified by pid calls RequestCPUrate (m, n).  This is a request for
 *	a fraction m/n of CPU time, effectively running on a CPU that is m/n
 *	of the rate of the actual CPU speed.  m of every n quantums should
 *	be allocated to the calling process.  Both m and n must be greater
 *	than zero, and m must be less than or equal to n.  MyRequestCPUrate
 *	should return 0 if successful, i.e., if such a request can be
 *	satisfied, otherwise it should return -1, i.e., error (including if
 *	m < 1, or n < 1, or m > n).  If MyRequestCPUrate fails, it should
 *	have no effect on scheduling of this or any other process, i.e., as
 *	if it were never called.
 */

int MyRequestCPUrate (pid, m, n)
	int pid;
	int m;
	int n;
{
   // Printf("in request");
    int i;
    if(m < 1 || n< 1 || m > n)
        return -1;
    
    for(i = 0; i < MAXPROCS; i++)
    {
        if(proctab[i].valid && proctab[i].pid == pid){
            break;
        }
    }
    if(proctab[i].request_flag == 1){
        cpu = proctab[i].request + cpu;
    }
    if(cpu < (double)m/(double)n-0.001)
        return -1;
    
    proctab[i].request = (double)m / (double)n;
    proctab[i].request_flag = 1;
    cpu = cpu - proctab[i].request;
    if(cpu < 0){
        cpu = 0;
    }
    
    unrequest_update();
	/* your code here */

	return (0);
}
int unrequest_update(){
    int i;
    int un_counter = 0;
    //Printf("unquest update");
    for (i = 0; i < MAXPROCS; i++) {
        if (proctab[i].valid && proctab[i].request_flag == 0) {
            un_counter++;
        }
    }
//    Printf("%d", un_counter);
//    Printf("%f",cpu);
    for (i = 0; i < MAXPROCS; i++) {
        if (proctab[i].valid && proctab[i].request_flag == 0) {
            proctab[i].request = cpu / un_counter;
        }
    }
    
    return 0;
}
