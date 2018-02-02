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
#define MAXELEMENTS 200

/* Define a struct model to define what data will be be
transmitted between threads and recursive function calls */
typedef struct {
  int* a;
  int first;
  int last;
} structArray;

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
int arrayOfElements[MAXELEMENTS]; /* Array to sort */
int count; /* Counter for the number of Threads created */
pthread_mutex_t thready; /* Locks for count */

void* quickSort(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  long l; /* use long in case of a 64-bit system */
  int i;
  structArray list;

  l = MAXELEMENTS; /* Set the number of elements in the array */
  count = 0;

  for(i = 0; i < l; i++){
    arrayOfElements[i] = rand()%1000;
  }

  #ifdef DEBUG
    printf("initial array/vector: \n");
    printf("[ ");
    for(i = 0; i < l; i++){
      printf("%d ", arrayOfElements[i]);
    }
    printf("]");
  #endif

  list.last = l - 1;
  list.first = 0;
  list.a = &arrayOfElements[0]; //First element in the array, first in the memmory

  /* Instantiate mutes/locks */
  pthread_mutex_init(&thready, NULL);

  start_time = read_timer();
  quickSort((void*) &list); /* Sort array */
  end_time = read_timer();
  #ifdef DEBUG
    printf("\nSorted array: \n");
    printf("[ ");
    for(i = 0; i < l; i++) {
      printf("%d ", arrayOfElements[i]);
    }
    printf("]\n\n");
  #endif
  printf("The execution time is %g sec\n", end_time - start_time);
}

void* quickSort(void *array) {
  structArray lArray, rArray; /* Two structs for a the two splits*/
  pthread_t ltid; /* New thread for sorting one split */
  int pivot, i_pivot, first, last, right, left, temp, *a;

  first = ((structArray *) array)->first; //First element
  last = ((structArray *) array)->last; //First element
  a = ((structArray *) array)->a; //Copy of Array.

  /* Base case fo the reccursive */
  if (first >= last) {
    return;
  }

  i_pivot = (first + last)/2;
  pivot = arrayOfElements[i_pivot]; //Get pivot element from the global array

  //quickSort
  left = first;
  right = last;
  while (left <= right) {
    if (pivot < a[left]) {
      temp = arrayOfElements[left];
      arrayOfElements[left] = a[right];
      arrayOfElements[right] = temp;
      if (right == i_pivot) {
        i_pivot = left;
      }
      right--;
    }
    else {
      left++;
    }
  }

  /*Place the pivot in its correct place (in the global array),
  no locks because every thread works in their own splits */
  temp = arrayOfElements[right];
  arrayOfElements[right] = pivot;
  arrayOfElements[i_pivot] = temp;

  //Prepare the two sub-lists with the new struct
  lArray.a = a;
  lArray.first = first;
  lArray.last = right-1;
  rArray.a = a;
  rArray.first = right + 1;
  rArray.last = last;
  //pthread_mutex_lock(&thready);
  //printf("Hello, I am thread nr.%d\n", ++count);
  //pthread_mutex_unlock(&thready);
  pthread_create(&ltid, NULL, quickSort, (void *) &lArray);
  //printf("quickSort\n");
  quickSort((void *) &rArray);
  pthread_join(ltid, NULL);
  //printf("join\n");
}
