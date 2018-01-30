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
#define MAXWORKERS 10   /* maximum number of workers */

int numWorkers;           /* number of workers */
#define MAXELEMENTS = 50;

struct array {
  int a*;
  int first;
  int last;
}

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
int arrayOfElements[MAXELEMENTS];

void *quickSort(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  long l; /* use long in case of a 64-bit system */
  int i;
  struct array list;

  long l = 0;
  for(i = 0; i < n; i++){
    arrayOfElements[i] = rand(1, 1000);
  }

  #ifdef DEBUG
    printf("initial array/vector: \n");
    printf("[");
    for(i = 0; i < MAXELEMENTS; i++){
      printf("%d, ", arrayOfElements[i]);
    }
    printf("]");
  #endif

  //list.a = &arrayOfElements[0];
  list.last = MAXELEMENTS - 1;
  list.first = 0;
  list.a = &arrayOfElements[0];

  start_time = read_timer();
  qucikSort(list);
  end_time = read_timer();
  #ifdef DEBUG
    printf("sorted array/vector: \n");
    printf("[");
    for(i = 0; i < MAXELEMENTS; i++){
      printf("%d, ", arrayOfElements[i]);
    }
    printf("]");
  #endif
  printf("The execution time is %g sec\n", end_time - start_time);
}

void *quickSort(void *array) {
  struct array lArray, rArray;
  pthread_t ltid;
  int pivot, i_pivot, first, last, left, temp;

  first = ((struct *) array)->first;
  last = ((struct *)) array)->last;
  a = ((struct *)) array)->a; //Copy of Array.

  if (first >= last) {
    return;
  }

  i_pivot = (first + last)/2;
  pivot = arrayOfElements[i_pivot];
  left = first;
  right = last;
  while (left <= right) {
    if (pivot < arrayOfElements[left]) {
      temp = a[left];
      a[left] = a[right];
      a[right] = temp;
      if (right == i_pivot) {
        i_pivot = left;
      }
      right--;
    }
    else {
      left++;
    }

  //Place the pivot in its correct place (in the global array)
  temp = arrayOfElements[right];
  arrayOfElements[right] = pivot;
  arrayOfElements[i_pivot] = temp;

  //Prepare the two sub-lists
  lArray.a = a;
  lArray.first = right + 1;
  lArray.last = last;
  rArray.a = a;
  rArray.first = first;
  rArray.last = right-1;
  pthread_create(&ltid, NULL, quickSort, (void *) lArray);
  quickSort(rArray);
  pthread_join(ltid, NULL);

  }
}
