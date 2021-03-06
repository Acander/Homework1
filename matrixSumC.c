/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

int numWorkers;           /* number of workers */
int numArrived = 0;       /* number who have arrived */

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size;
pthread_mutex_t sumLock, maxLock, minLock, bagLock;
int globalSum, globalMax, globalMaxi, globalMaxj, globalMin, globalMini, globalMinj;
int matrix[MAXSIZE][MAXSIZE]; /* matrix */
int bagOfTasks; /* A bag of tasks from which the threads will pull rows from*/

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /*initialize all mutexs*/
  pthread_mutex_init(&sumLock, NULL);
  pthread_mutex_init(&maxLock, NULL);
  pthread_mutex_init(&minLock, NULL);
  pthread_mutex_init(&bagLock, NULL);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%1000;
	  }
  }
  /*initialize the global variables*/
  globalMin = matrix[0][0];
  globalMax = matrix[0][0];
  bagOfTasks = 0;

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  for (i = 0; i < numWorkers; i++)
    pthread_join(workerid[i], NULL);

    /* get end time */
  end_time = read_timer();
  /* print results */
  printf("The total is %d\n", globalSum);
  printf("The maximum value is %d\n", globalMax);
  printf("Index: row %d, column %d\n", globalMaxi, globalMaxj);
  printf("The minimum value is %d\n", globalMin);
  printf("Index: row %d, column %d\n", globalMini, globalMinj);
  printf("The execution time is %g sec\n", end_time - start_time);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int total, i, j, first, last, maxValue, maxi, maxj, minValue, mini, minj, task;
  bool done = false;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  total = 0;
  maxValue = matrix[0][0];
  maxi = 0;
  maxj = 0;
  minValue = matrix[0][0];
  mini = 0;
  minj = 0;

  while (!done) {
    /* Grab the next row from the bag of tasks. Every
    thread work as much as they can, unlike spliting up the work
    before running*/
    pthread_mutex_lock(&bagLock);
      task = bagOfTasks++;
    pthread_mutex_unlock(&bagLock);
    if (task < size) done = true;

    for (j = 0; j < size; j++){
      total += matrix[task][j];
      if (maxValue < matrix[task][j]){
        maxValue = matrix[task][j];
        maxi = task;
        maxj = j;
      }
      if (minValue > matrix[task][j]){
        minValue = matrix[task][j];
        mini = task;
        minj = j;
      }
    }
  }

  pthread_mutex_lock(&sumLock);
  globalSum += total;
  pthread_mutex_unlock(&sumLock);

  if (maxValue > globalMax) {
    pthread_mutex_lock(&maxLock);
      if (maxValue > globalMax){
        globalMax = maxValue;
        globalMaxi = maxi;
        globalMaxj = maxj;
      }
      pthread_mutex_unlock(&maxLock);
      }

  if (minValue < globalMin) {
    printf("Printing shit now\n");
    pthread_mutex_lock(&minLock);
    if (minValue < globalMin){
      globalMin = minValue;
      globalMini = mini;
      globalMinj = minj;
    }
    pthread_mutex_unlock(&minLock);
  }

}
