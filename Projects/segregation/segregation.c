
#define _BSD_SOURCE
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/select.h>


#include "check_input.h"
#include "display.h"

   
   
   
   float   calcSatisfaction(char token,int row,int col, int size, char[][size]);
 
   void    printBoardNum(int,int,float,int size, char [][size]);
   void    printBoardInt(bool,int,int,float,int size, char [][size]);
 
   void    move(int,int,int size, char [][size]);
   float   calcSatValue(int,int,int size, char personGrid[][size]);	
   void    printInfo(int,int,int,int);
   int     cycle(bool,bool,int,int,int,int,int size, char personGrid[][size]);
   void    resetBoard(int,int,int size,char personGrid[][size]);

   bool    checkVals(int,int,int,int);
/*
	Main Program

	Takes in command line arguments to determine
	the inital state. The board is instantiated
	and runs in one of two modes.
	
	INTERACTIVE: Program allows user inputs to 
	modify state of city

	PRINT: Prints a number of cycles defined by
	command inputs
	
*/

	int main(int argc, char *argv[])
   {	 
	  
	  //Command line arguments

	  int vals[4];
	  int size=5,sat=0,vac=0,prop=0,printNum=-1,valCount = 0;
	  bool isPrint=false;
	  for(int index = 1; index < argc; ++index)
	  {
		if(argv[index][0]=='-')
		{
			isPrint = true;
			int len = strlen(argv[index]);
			if(len==2)
			{
				
				printNum = atoi(argv[index+1]);
				index++;
			}
			else
			{
				char theNum[len-2];
				strncpy(theNum,argv[index]+2,len-2);	
				printNum = atoi(theNum);	
			}
		}
		else
		{
			vals[valCount] = atoi(argv[index]);
			valCount++;
		}
	  }

	  size = vals[0];
	  sat  = vals[1];
	  vac  = vals[2];
	  prop = vals[3];
	  	
	  //Check Command Line arguments
	  bool cmdValid = checkVals(size,sat,vac,prop);
          
	  if(cmdValid == false)
	  {
		printf("usage: segregation [-NP] size satisfaction vacancy proportion\n");
		return 0;
	  } 



	  //Instantiate the Board
	  clear();
      	  char personGrid[size][size];
	  resetBoard(vac,prop,size,personGrid);
		
          int moves = 0;

	  //----------------- PRINT MODE --------------------
	  if(isPrint)
	  {
             printBoardNum(0,moves,calcSatValue(vac,prop,size,personGrid),size,personGrid);	
	     printInfo(size,sat,vac,prop); 
	     for(int i = 1;i<printNum;++i)
	     {
		moves = cycle(true,false,i,sat,vac,prop,size,personGrid);
		printInfo(size,sat,vac,prop);
 	     }
	  }

	  //---------------- INTERACTIVE MODE --------------
	  else
          {
	        printBoardInt(true,0,moves,calcSatValue(vac,prop,size,personGrid),size,personGrid);  	
		int  count   =  1     ;
		bool isGoing =  true  ;
		while(1)
		{
		        printf("(/)stop/go, (.)step, (r)eset, (s)at N, (v)ac N, (p)rop N, (q)uit,(i)nfo\n(h)elp\n>"); 
			fflush(stdout); 
			char buf[25];
			int input = check_input(stdin,buf,25,4);		
			if(input < 0)
			{
				perror( "select" );
				continue;
			}
			int indexOfSpace = -1;
			for(int ind = 0; ind < 25;ind++)
			{
				if(buf[ind] == ' ')
				{
					indexOfSpace = ind;
					break;
				}
			}
			
			char firstToken = buf[0];
			if((firstToken == 's' || firstToken == 'p' || firstToken == 'v') && indexOfSpace== -1)continue;
			char val[24-indexOfSpace];
			strncpy(val,buf+indexOfSpace+1,24-indexOfSpace);
			int valInt = atoi(val);
			switch(buf[0])
			{
			case '/':
				isGoing = !(isGoing);	        	
				break;
			case '.':
				count++;
				moves = cycle(isGoing,true,count,sat,vac,prop,size,personGrid);
				
				continue;
			case 'r':
				resetBoard(vac,prop,size,personGrid);
				count = 0;
				moves = 0;
				printBoardInt(isGoing,count,moves,calcSatValue(vac,prop,size,personGrid),size,personGrid);
				continue;
			case 's':
					if(valInt <= 99 && valInt >= 1)
					{
						sat = valInt;	
					}
				break;
			case 'v':
					if(valInt <100 && valInt>0)
					{
						vac = valInt;
						resetBoard(vac,prop,size,personGrid);
						count = 0; moves = 0;
						printBoardInt(isGoing,count,moves,calcSatValue(vac,prop,size,personGrid),size,personGrid);
					}	
				continue;
			case 'p':
				        if(valInt < 100 && valInt > 0)
					{
						prop = valInt;
						resetBoard(vac,prop,size,personGrid);
						count = 0; moves = 0;
						printBoardInt(isGoing,count,moves,calcSatValue(vac,prop,size,personGrid),size,personGrid);
					}
				continue;
			case 'q':
				return 0;
			case 'h':
				set_cur_pos(size+8,0);
				printf("(/)stop/go, (.)step, (r)eset,(s)at N, (v)ac N, (p)rop N, (q)uit, (i)nfo,\n(h)elp");
				fflush(stdout);
				continue;
			case 'i':
				set_cur_pos(size+8,0);
				printf("size %d, satisfaction %.2f, vacancy %.2f, proportion %.2f\n",size,sat/100.0,vac/100.0,prop/100.0);
				fflush(stdout);
				continue;
			default:
				//printf("usage: incorrect input. Try again...\n");	
				break;
			}

			if(isGoing)
			{
				moves = cycle(isGoing,true,count,sat,vac,prop,size,personGrid);
				count++;
			}
			else
			{
				printBoardInt(isGoing,count,moves,calcSatValue(vac,prop,size,personGrid),size,personGrid);
			}	
		}
	  } 
         	  return 0;
   }	
				
   

/*
//	CALCSATISFACTION
//
//	Checks non-vacant spots surrounding a token and
//	calculates its relative satisfaction. This
//	assumes that having zero neighbors yields 
//	perfect satisfaction.
*/

   float calcSatisfaction(char token, int row,int col,int size, char personBoard[][size])
   {
	int satisfaction = 0;
	char nullToken = '.';
	if(token == '.')
		return 0;
			int numNear=0;
			   if(row != 0 && col != 0 && personBoard[row-1][col-1] != nullToken)
			   { 
				numNear++;
			   	if(personBoard[row - 1][col - 1] == token)
			   	{
                                    satisfaction++;
			   	}
                           } 
                           if(row != 0 && col != size-1 && personBoard[row-1][row+1] != nullToken)
			   {
				numNear++;
				if( personBoard[row - 1][col + 1] == token)
			   	{
                                  satisfaction++;
			   	}
			   }
                           if(row != size-1 && col !=0 && personBoard[row+1][col-1] != nullToken)
			   {
				numNear++;
				if( personBoard[row + 1][col - 1] == token)
			   	{
                                  satisfaction++;
			   	}
			   }
                           if(row != size-1 && col != size-1 && personBoard[row+1][col+1] != nullToken)
			   {
				numNear++;
				if( personBoard[row + 1][col + 1] == token)
                           	{       
				  satisfaction++; 
			   	}
			   }	
			   
			   if(row != 0 && personBoard[row-1][col] != nullToken)
			   {
				numNear++;
				if(personBoard[row-1][col] == token)satisfaction++;
			   }
			   if(row != size-1 && personBoard[row+1][col] != nullToken)
			   {
				numNear++;
				if(personBoard[row+1][col] == token)satisfaction++;
			   }
			   if(col != 0 && personBoard[row][col-1] != nullToken)
			   {
				numNear++;
				if(personBoard[row][col-1] == token)satisfaction++;
    			   }
			   if(col != size-1 && personBoard[row][col+1] != nullToken)
			   {
				numNear++;
				if(personBoard[row][col+1] == token)satisfaction++;
			   }

    if(numNear == 0)return 1.0;
    return (float)satisfaction/numNear;

   }


/*
//	PRINTBOARDNUM
//	
//	Uses basic standard library commands to print
//	the non-interactive version of the game state.
//	This requires no cursor manipulation.
*/
   void printBoardNum(int cycle, int moves,float satVal,int size, char Life[][size])
   {
	
	for(int row =0 ; row<size; row++)
        {
        	for(int col = 0; col<size; col++)
                {
                    printf("%c",Life[row][col]);	
                }
                printf("\n");
        }
	printf("cycle: %d\nmoves this cycle: %d\nsegregation measure: %.4f\n",cycle,moves,satVal);
	return;
   }
  
/*
//	MOVE
//	
//	For a chose person at (row,col), it finds a random space
//	That is not occupied, exchanging the two spaces.
*/ 
   void move(int row, int col,int size, char personBoard[][size])
   {
    	char token = personBoard[row][col];
    	srand(time(NULL));
    	while(1)
    	{
		int r = rand() % size;
		int c = rand() % size;
		if(personBoard[r][c] == '.')
		{
			personBoard[r][c] = token;
			personBoard[row][col] = '.';
			return;
		}
    	}
	
//    PUTS TOKEN  AT FRONT
//
//    for(int r = 0;r < size;++r)
//    {
//	for(int c = 0;c<size;++c)
//	{
//            if(personBoard[r][c] == '.')
//            {
// 	        personBoard[r][c] = token;
//	        personBoard[row][col] = '.';
//	        return;
//            }  
//        }
//     }
    
    return;
   }


/*
//	CALCSATVALUE
//	
//	Sums up the total satisfaction for each token and divides by
//	The total number of vacant tokens, yielding a Segregation
//	value.
*/ 
   float calcSatValue(int vac,int prop, int size, char personGrid[][size])
   {
	float satNumer = 0;
	int vacSpace  =  size*size*vac/100;
	int richSpace =  (size*size-vacSpace)*prop/100;
	int poorSpace =  size*size-richSpace-vacSpace;
	for(int i = 0;i<size;++i)
	{
		for(int j = 0;j<size;++j)
		{
			satNumer+=calcSatisfaction(personGrid[i][j],i,j,size,personGrid);
		}
	}
	float satVal = satNumer/(richSpace+poorSpace);
	return satVal;
   }

/*
//	CYCLE
//
//	Single cycle of state, giving a single state change. Calculates
//	and returns the number of moves for this cycle.
*/
   int cycle(bool isGoing, bool isInteractive,int count,int sat, int vac, int prop,int size, char personGrid[][size])
   {
	int thisMoves = 0;
	for(int i = 0; i<size;++i)
	{
		for(int j = 0; j<size; ++j)
		{
			if(sat/100.0 > calcSatisfaction(personGrid[i][j],i,j,size,personGrid))
			{
				if(personGrid[i][j] != '.')
				{
					thisMoves++;
					move(i,j,size,personGrid);
				}
			}
		}
	}
	float satVal = calcSatValue(vac,prop,size,personGrid);
	if(isInteractive)
	{
		printBoardInt(isGoing,count,thisMoves,satVal,size,personGrid);
	}
	else
	{
		printBoardNum(count,thisMoves,satVal,size,personGrid);
	}
	
	return thisMoves;	
   }

/*
//	PRINTBOARDINT
//
//	The printer for the interactive mode. Prints cycles using
//	cursor methods to reprint on the same line.
*/
   void printBoardInt(bool isGoing,int count,int moves,float satVal,int size,char personGrid[][size])
   {
	clear();
	for(int i = 1; i<=size; ++i)
	{
		for(int j = 0; j<size; ++j)
		{
			set_cur_pos(i,j);
			put(personGrid[i-1][j]);
		}
	}
	char s[] = "paused";
	char t[] = "playing";
	set_cur_pos(size+1,0); 
	if(isGoing)
		printf("cycle: %d\nmoves this cycle: %d\nsegregation measure: %.4f\n\nstate: %s\n",count,moves,satVal,t);
	else
		printf("cycle: %d\nmoves this cycle: %d\nsegregation measure: %.4f\n\nstate: %s\n",count,moves,satVal,s);
	fflush(stdout);

	return;
   }
 
/*
//	PRINTINFO
//
//	Print the information gathered from command line
//	inputs and prints to standard output
*/
   void printInfo(int size, int sat, int vac, int prop)
   {
	printf("size %d, satisfaction %.2f, vacancy %.2f, proportion %.2f\n",size,sat/100.0,vac/100.0,prop/100.0);
	return;
   }

/*
//	RESETBOARD
//	
//	Uses vacancy and proportion arguments
//	to create and initial state of the
//	board.
*/
   void resetBoard(int vac, int prop,int size, char personBoard[][size])
   {
	for(int i = 0;i<size;++i)
	{
		for(int j = 0;j<size;++j)
		{
			personBoard[i][j] = '.';
		}
	}
	int numVac   =  size*size*vac/100;
	int numRich  =  (size*size-numVac)*prop/100;
	int numPoor  =  size*size - numVac - numRich;
	srand(41);
        int tempRich = numRich,tempPoor = numPoor;
	
	while(tempRich !=0 || tempPoor !=0)
	{
		int row = rand() % size;
		int col = rand() % size;
		
		if(tempRich !=0 && personBoard[row][col] == '.')
		{
			personBoard[row][col] = '+';
			tempRich--;
		}
		else if(tempPoor !=0 && personBoard[row][col] == '.')
		{
			personBoard[row][col] = '-';
			tempPoor--;
		}
	}
	return;
   }

/*
//	CHECKVALS
//	
//	Runs a quick check of each command-line argument
//	to make sure they are within the correct range.
*/
   bool checkVals(int size, int sat, int vac, int prop)
   {
	
	if( size < 5 || size > 39 )
	{
		printf("size (%d) is not between 5 and 39\n",size);
		return false;
	}
	if( sat  < 1 || sat  > 99 )
	{
	        printf("satisfaction (%d) is not between 1 and 99\n",sat); 	
		return false;
	}
	if( vac  < 1 || vac  > 99 )
	{	
		printf("vacancy (%d) is not between 1 and 99\n",vac);
		return false;
	}
	if( prop < 1 || prop > 99 )
	{
		printf("proportion (%d) is not between 1 and 99\n",prop);
		return false;
	}

	return true;
   }

//===================== END OF SEGREGATION.C ===============================
