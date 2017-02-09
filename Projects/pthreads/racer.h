// racer.h - a thread controlling a small figure that races
//		across a character window
// @author James Heliotis
// @contributor tjb


#ifndef _RACER_H
#define _RACER_H

#include <stdlib.h>
#include <stdio.h>

#define FINISH_LINE 70
#define MAX_NAME_LEN 9    // not including the trailing NUL byte
#define DEFAULT_WAIT 200  // default wait time 

// Rcr struct 
typedef struct Rcr {

    // current distance from starting line
    //
    int dist;

    // vertical position of the racer, i.e. "racing lane"
    //
    int row;

    // graphic: the drawable text
    //
    char *graphic;

} Racer;

// initRacers - Do setup work for all racers at the start of the program.
// @param milliseconds length of pause between steps in animation 
//
void initRacers( long milliseconds );

// makeRacer - Create a new racer.
//
// @param name the string name to show on the display for this racer
// @param position the row in which to race
// @return Racer pointer a dynamically allocated Racer object
// @pre strlen( name ) < MAX_NAME_LEN, for display reasons.
//
Racer *makeRacer( char *name, int position );

// destroyRacer - Destroy all dynamically allocated storage for a racer.
//
// @param racer the object to be de-allocated
//
void destroyRacer( Racer *racer );

// run Run one racer in the race.
// Initialize the display of the racer*:
//   The racer starts at the start position, column 1.
//   The racer's graphic (text name ) is displayed.
// This action happens repetitively, until its position is at FINISH_LINE:
//   Randomly calculate a waiting period, no more than
//   the value given to initRacers
//   Sleep for that length of time.
//   Change the display position of this racer by +1 column*:
//     Erase the racer's name from the display.
//     Update the racer's dist field by +1.
//     Display the racer's name at the new position.
//
// The intention is to execute this function many times simultaneously,
// each in its own thread.
//
// note: Care is taken to keep the update of the display by one racer "atomic".
//
//
// @pre racer cannot be NULL.
// @param racer a Racer, declared as void* to be comptable with pthread
//		  interface
// @return void pointer to  
//
void *run( void *racer );

#endif
