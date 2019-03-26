/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"

//Scheduler structure, used to define a scheduler object.
typedef struct _scheduler {
  scheme_t algorithm;   //Scheduler algorithm to use.
  priqueue_t q;         //Job queue.
  priqueue_t finished_q;

  job_t** cores; //Points to the cores currently running jobs.
  int numCores;         //Number of cores to use.
  int lastUpdated;

} scheduler_t;

scheduler_t* sched;
int (*cmp)(const void*, const void*);

//Create a job
job_t* createJob(int jobID, int priority, int arrivalTime, int remainingTime) {
 job_t* j = malloc(sizeof(job_t));
 j->jobID = jobID;
 j->priority =  priority;
 j->timeArrived = arrivalTime;
 j->timeRemaining = remainingTime;
 j->timeNeeded = remainingTime;
 j->timeStart = -1;
 return j;
}

/* Below, we define our compare functions for each respective scheduling algorithm */

//First Come First Serve
int cmpFCFS (const void * elem1, const void * elem2){
	 //Cast the elements as jobs
	 job_t* job1 = (job_t *) elem1;
	 job_t* job2 = (job_t *) elem2;

	 // 0 - Equivalent priority | <0 - elem1 has higher priority | >0 - elem2 has higher priority.
	 return (job1->timeArrived - job2->timeArrived);
}

//Shortest Job First
int cmpSJF (const void * elem1, const void * elem2){
	//Cast the elements as jobs
	job_t* job1 = (job_t *) elem1;
	job_t* job2 = (job_t *) elem2;

	// 0 - Equivalent priority | <0 - elem1 has higher priority | >0 - elem2 has higher priority.
	//use FCFS to dictate tiebreakers.
	int diff = job1->timeRemaining - job2->timeRemaining;
	return (diff == 0) ? cmpFCFS(elem1, elem2) : diff;
}

//Job Priority
int cmpPRI (const void * elem1, const void * elem2){
	//Cast the elements as jobs
	job_t* job1 = (job_t *) elem1;
	job_t* job2 = (job_t *) elem2;

	int diff = job1->priority - job2->priority;
	return (diff == 0) ? cmpFCFS(elem1, elem2) : diff;
}

//Round Robin
int cmpRR (const void * elem1, const void * elem2){
	return 1;
}

/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler.
		These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used.
		This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
	//Initialize scheduler object with provided parameters.
	sched = malloc(sizeof(scheduler_t));
	sched->numCores = cores;
	sched->algorithm = scheme;
  sched->lastUpdated = 0;
	//Array initialization
	sched->cores = malloc(sched->numCores * sizeof(job_t*));
	for (int i = 0; i < sched->numCores; i++) sched->cores[i] = NULL;

	//To initialize the queue, we need to provide the appropriate compare function.
	switch(sched->algorithm) {
		case FCFS: cmp = &cmpFCFS; break;
		case SJF:  cmp = &cmpSJF;  break;
		case PSJF: cmp = &cmpSJF;  break;
		case PRI:  cmp = &cmpPRI;  break;
		case PPRI: cmp = &cmpPRI;  break;
		case RR:   cmp = &cmpRR;   break;
	}
	priqueue_init(&sched->q, cmp);
  priqueue_init(&sched->finished_q, cmp);
}


/**
  Called when a new job arrives.

  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made.

 */

int scheduler_new_job(int job_number, int job_time, int running_time, int priority)
{
  printf("new job function\n");
	//Create the new job
	job_t* j = createJob(job_number, priority, job_time, running_time);

	int core = -1;
	for (core = 0; core < sched->numCores; core++){
		if (!sched->cores[core]) {
			sched->cores[core] = j;
			j->timeStart = job_time;
			return core;
		}
	}

	//Assume there are no free cores. Check for preemption.
	core = -1; //reset
	if(sched->algorithm==PSJF || sched->algorithm==PPRI) {
		job_t* lowestPriJob = j;
		for (core = 0; core < sched->numCores; core++) {
			if (cmp(lowestPriJob, sched->cores[core]) < 0) {
				lowestPriJob = sched->cores[core];
				break;
			}
		}
		if(sched->cores[core]->timeStart == job_time) sched->cores[core]->timeStart = -1; //reset time
		j->timeStart = job_time;	//swap job on queue with job in core
		priqueue_offer(&sched->q, sched->cores[core]);
		sched->cores[core] = j;
	}
	if(core >= 0) return core; 	//we have a core
	j->timeStart = -1;
	priqueue_offer(&sched->q, j); //add to queue
	return -1;
}


/**
  Called when a job has completed execution.

  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.

  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int job_time)
{
  //sched->last I updated it.
  //for
  // if this is an active core,
  // then cores-> time left -= job_time - sched-> last time I updated.
//sched->lastUpdated

	//Perform calculations
	for(int i=0; i < sched->numCores; i++)
		if(sched->cores[i]) sched->cores[i]->timeRemaining -= (job_time - sched->lastUpdated);

	sched->lastUpdated = job_time;	//Record last updated time

	//Record time finished, add to finished queue, and free the core.
	sched->cores[core_id]->timeFinished = job_time;
  priqueue_offer(&sched->finished_q, sched->cores[core_id]);
  sched->cores[core_id] = NULL;
  int job = -1;
	if(priqueue_size(&sched->q) > 0)
  {
    job_t* temp = priqueue_poll(&sched->q);
    if(temp->timeStart == -1) temp->timeStart = job_time;
		sched->cores[core_id] = temp;

	  job = temp->jobID;
	}
	return job;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.

  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int job_time)
{
	job_t* coreJob = sched->cores[core_id];
  int jobID = -1;
	if(!coreJob){
		if(priqueue_size(&sched->q) == 0) return -1; //could not be scheduled
    //else continue past next else...
	}
	else {
		priqueue_offer(&sched->q, coreJob);
	}

	//move from queue to core
  job_t * j = priqueue_poll(&sched->q);
  if(j)
  {
    jobID = j->jobID;
    if(j->timeStart == -1)
      j->timeStart = job_time;
    sched->cores[core_id] = j;
  }

  //if(coreJob) -> true
  //
	// if(cores[core_id]->timeStart == -1) {
	// 	cores[core_id]->timeStart = job_time;
	// }
	return jobID;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
  double time = 0.0;
  int size = priqueue_size(&sched->finished_q);
  for(int i=0; i < size; i++){
    job_t * j = priqueue_at(&sched->finished_q, i);
    time += j->timeFinished - j->timeArrived - j->timeNeeded;
  }
  return time / size;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
  //priqueue_t finished_q
  double time = 0.0;
  int size = priqueue_size(&sched->finished_q);
	for(int i=0; i < size; i++){
    job_t * j = priqueue_at(&sched->finished_q, i);
    time += j->timeFinished - j->timeArrived;
	}

	return time / (size*1.0);
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
    //priqueue_t finished_q
    double time = 0.0;
    int size = priqueue_size(&sched->finished_q);
  	for(int i=0; i < size; i++){
      job_t * j = priqueue_at(&sched->finished_q, i);
      time += j->timeStart - j->timeArrived;
  	}

  	return time / (size*1.0);
}


/**
  Free any memory associated with your scheduler.

  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
	// for (int i = 0; i < ; i++) if (!sched->cores[i]) free(sched->cores[i]);
	// free(cores);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
  for(int i = 0; i < sched->numCores; i++)
  {
    job_t* j = sched->cores[i];
    if(j)
    {
      if(sched->algorithm == RR) printf("%d(-1) ", j->jobID);
      else printf("%d(%d) ", j->jobID, j->priority);
    }
  }

  for(int i = 0; i < priqueue_size(&sched->q); i++)
  {
    job_t* j = priqueue_at(&sched->q, i);
    if(j) {
      if(sched->algorithm == RR)
        printf("%d(-1) ", j->jobID);
      else
        printf("%d(%d) ", j->jobID, j->priority);
    }
  }

}
