 /*
 *  Author: Luke Shadler
 *	racer.c produces a single threaded racer
 */
#define _POSIX_C_SOURCE 199309L
#define _POSIX_SOURCE
#define _BSD_SOURCE
#include <unistd.h>
#include <pthread.h>
#include "racer.h"
#include "display.h"
#include <time.h>
long maxWait;
pthread_mutex_t racer_mutex;

// initRacers - Setup all racers at start of program
// @param milliseconds length of pause between steps
//
void initRacers( long milliseconds )
{
	clear();
	maxWait = milliseconds;	
	pthread_mutex_init(&racer_mutex,NULL);
}

// makeRacer - creates a new racer from conditions
//
// @param name the string name displayed for this user
// @param position the row that this racer will be on
// @return new racer pointer to dynamically allocated Racer
//
Racer *makeRacer( char *name, int position )
{
	Racer *racer = (Racer *)malloc( sizeof(Racer) );
	racer->graphic  = name;
	racer->dist = 1;
	racer->row  = position;
	return racer;
}

// destroyRacer - Free all dynamically allocated storage for racer
//
// @param racer Racer object to destroy
//
void destroyRacer( Racer *racer)
{
	free(racer->graphic);
	free(racer);
}


// run - increments distance and displays racer while it hasnt passed the finish line
//
// @param racer the racer you want to modify
// @return NULL void pointer
//
void *run( void *racer )
{
	struct timespec start = {0,0},finish = {0,0};
	clock_gettime(CLOCK_MONOTONIC,&start);
	Racer *rc = (Racer *)racer;
	while(rc->dist < FINISH_LINE)
	{		

		pthread_mutex_lock(&racer_mutex);
		set_cur_pos(rc->row,rc->dist);
	    printf("%c[2K",27);						//this line was looked up to clear one line	
		printf("%s",rc->graphic);
		fflush(stdout);
		pthread_mutex_unlock(&racer_mutex);
	
		int wait = rand() % maxWait;
		usleep(wait * 1000);
		rc->dist++;
	}
	clock_gettime(CLOCK_MONOTONIC,&finish);
	double time_tot = -((double)start.tv_sec + 1.0e-9*start.tv_nsec) + 
					   ((double)finish.tv_sec + 1.0e-9*finish.tv_nsec);
	set_cur_pos(rc->row,rc->dist + 12);
	printf("%f",time_tot);
	fflush(stdout);
	pthread_exit(NULL);
	return NULL;
}













