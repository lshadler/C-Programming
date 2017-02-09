#define _BSD_SOURCE
#define _POSIX_SOURCE
#include "racer.h"
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <ctype.h>
#include "display.h"
#include <time.h>
bool isaNumber(char str[]);
 

/// main creates threads based on command-line named racers
///
/// @return exit code (1 if abnormal exit, 0 if normal)
int main(int argc, char *argv[])
{
	int numRc = argc-1;
	if(argc > 1)
	{
		long milliseconds = DEFAULT_WAIT;
		int add = 0;
		if(isaNumber(argv[1]))
		{
			add = 1;
			numRc--;
			milliseconds = strtol(argv[1],NULL,0);
		}
	    if(argc == 1+add)
	    {
	    	fprintf(stderr,"Usage: pt-cruisers [random-seed] names...\n");
	    }
		Racer **racers = malloc(sizeof(Racer) * numRc);
		
		//---------- Make Racers -----------------------
		for(int i = 1+add; i < argc; i++)
		{
			char *tempName = argv[i];
			char *name = malloc(MAX_NAME_LEN+1);
			int len = strlen(tempName);
			if(len > MAX_NAME_LEN)
			{
				fprintf(stderr,"Usage: racer names must not exceed length 9.\n");
				free(racers);
				return 1;
			}
			else
			{
				int numLead  = (MAX_NAME_LEN-len)/2;
				int numTrail = MAX_NAME_LEN-len-numLead;
				
				for(int ind = 0; ind < numLead; ++ind)
				{
					strcat(name,"_");
				}
				strcat(name,tempName);
				for(int ind2 = 0; ind2 < numTrail; ++ind2)
				{
					strcat(name,"_");
				}

			}
			racers[i-1-add] = makeRacer(name,i-add);
		}
        
        //------------ Do Stuff ------------------------------
		srand(time(NULL));
		initRacers(milliseconds);
		pthread_t threads[numRc];
		for(int thisRc = 0; thisRc < numRc; ++thisRc)
		{
			int sc = pthread_create(&threads[thisRc],
								NULL,
								run,
								racers[thisRc]);
			if(sc)
			{
				fprintf(stderr,"error: pthread_create\n");
				free(racers);
				return 1;
			}
		}
		for(int thd = 0; thd < numRc; ++ thd)
		{
			int *status, errc;
			errc = pthread_join(threads[thd],(void *) &status);
			if(errc)
			{
				fprintf(stderr,"error: pthread_join\n");
				free(racers);
				return 1;
			}
		}



        //------ Destroy Racers ----------------------
		for(int j = 0; j < numRc; ++j)
		{
			destroyRacer(racers[j]);
		}
		free(racers);
	
		set_cur_pos(numRc+1,0);
		return 0;

	}
	else
	{
	   	fprintf(stderr,"Usage: pt-cruisers [random-seed] names...\n");
		return 1;	
	}
}


/// isaNumber determines whether or not a string is a number
/// @param str string that is possibly a number
/// @return the boolean (true if a number)
bool isaNumber(char str[])
{
	for(size_t index = 0; index < strlen(str); ++index)
	{
		if(!isdigit(str[index]))
			return false;
	}
	return true;
}
