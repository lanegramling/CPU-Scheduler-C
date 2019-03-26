/** @file libscheduler.h
 */

#ifndef LIBSCHEDULER_H_
#define LIBSCHEDULER_H_

/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements.
*/
//Jobs store the following parameters which will be used to calculate the appropriate
//   values sought by the scheduler methods.
typedef struct _job_t {
    int priority;       //Priority of the job (lower value <=> higher priority)
    int jobID;          //Unique Job ID

    int timeArrived;    //Time of arrival of the job
    int timeStart;      //Time the job was started
    int timeRemaining;  //Time remaining on the job
    int timeFinished;   //Time the job was finished

    int timeNeeded;     //Needed time
    int timeWaiting;    //Time spent waiting

} job_t;



/**
  Constants which represent the different scheduling algorithms
*/
typedef enum {FCFS = 0, SJF, PSJF, PRI, PPRI, RR} scheme_t;

void  scheduler_start_up               (int cores, scheme_t scheme);
int   scheduler_new_job                (int job_number, int job_time, int running_time, int priority);
int   scheduler_job_finished           (int core_id, int job_number, int job_time);
int   scheduler_quantum_expired        (int core_id, int job_time);
float scheduler_average_turnaround_time();
float scheduler_average_waiting_time   ();
float scheduler_average_response_time  ();
void  scheduler_clean_up               ();

void  scheduler_show_queue             ();

#endif /* LIBSCHEDULER_H_ */
