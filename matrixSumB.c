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
int size, stripSize;  /* assume size is multiple of numWorkers */
pthread_mutex_t sumLock, maxLock, minLock;
int globalSum, globalMax, globalMaxi, globalMaxj, globalMin, globalMini, globalMinj;
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

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

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%100;
	  }
  }

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
  printf("Index row %d, column %d\n", globalMini, globalMinj);
  printf("The execution time is %g sec\n", end_time - start_time);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int total, i, j, first, last, maxValue, maxi, maxj, minValue, mini, minj;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* sum values in my strip */
  total = 0;
  maxValue = matrix[first][0];
  maxi = first;
  maxj = 0;
  minValue = matrix[first][0];
  mini = first;
  minj = 0;
  for (i = first; i <= last; i++)
    for (j = 0; j < size; j++){
      total += matrix[i][j];
      if (maxValue < matrix[i][j]){
        maxValue = matrix[i][j];
        maxi = i;
        maxj = j;
      }
      if (minValue > matrix[i][j]){
        minValue = matrix[i][j];
        mini = i;
        minj = j;
      }
  }
    pthread_mutex_lock(&sumLock);
      globalSum += total;
    pthread_mutex_unlock(&sumLock);

    if (maxValue > globalMax) {
      pthread_mutex_lock(&maxLock);
        //if (maxValue > globalMax){
          globalMax = maxValue;
          globalMaxi = maxi;
          globalMaxj = maxj;
        //}
      pthread_mutex_unlock(&maxLock);
    }

    if (minValue < globalMin) {
      pthread_mutex_lock(&minLock);
        //if (minValue < globalMin){
          globalMin = maxValue;
          globalMini = mini;
          globalMinj = minj;
        //}
      pthread_mutex_unlock(&minLock);
    }
}
