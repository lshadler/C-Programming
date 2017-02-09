// Display functions put, set_cur_pos, and clear were copied from RIT derived program display.c
#define _BSD_SOURCE
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void put(char);
void set_cur_pos(int,int);
void clear();
int calcNeighbors(int, int, int size, char [][size]);
void header(void);
void checkRules(int size, char [][size]);
void printBoard(int size, int count, char [][size]);	

/*
	Main Program

	Takes in command line arguments to determine
	the grid size. The board is instantiated
	and an infinite loop of checking the game
	rules, printing the board, and adding a time
	delay is created. Can only terminate with an
	escape command <Ctl + C>
*/

int main(int argc, char *argv[])
{	 
    int orgs, size, i, row, col, count = 0;

    header();

	printf("\nPlease enter the initial number of organisms: ");
	scanf("%i", &orgs);
        printf("\n");
	if(argc >1)
	{
		size = (int)*argv[1];
	}
	else
    {
        size = 20;
    }
  	printf("\n");
  	srand( 2*size );
    char Life[size][size];
	clear();
	for(i = 0; i<orgs; i++)
	{
	   row = rand();
	   row %= size;
	   col = rand();
	   col %= size;
	   Life[row][col] = '*';
	}
	
	for(row = 0; row<size; row++)
	{
	    for(col = 0; col<size; col++)
	    {
	   	    if(Life[row][col] != '*')
	      	    Life[row][col] = ' ';
	    }
	} 
	while ( 1 ) 
    {
	    checkRules(size,Life);
	        printBoard(size,count,Life);

	    usleep( 81000 );
                   count++;		        
	}
	 
    clear();	
    return 0;
}
 		
 			
void header(void) /*function for program header*/
{
   printf("\n\t..Welcome to the Game of Life..\n");
}


/*
	checkRules(int, char [][])

	Checks the board for the rules of
	the game of life. 
	A '*' survives if it has 2 or 3
	neighbors
	A ' ' is reborn if it has 3 
	neighbors
	Otherwise, the space should be
	made blank
*/	
void checkRules(int size,char Life[][size])
{
   int row, col;
   for(row = 1; row<size-1; row++)
   {
 	 for(col = 1; col<size-1; col++)
 	 {
 		   int neighbors = calcNeighbors(row,col,size,Life);

 		   if(Life[row][col] == '*' && (neighbors<2 || neighbors>=4))
 			  Life[row][col] = ' ';
                        if(Life[row][col] == ' ' && neighbors == 3)
 			  Life[row][col] = '*';	
 	}
   }
   return;
}

/*
	calcNeighbors

	Calculates the amount of neighbors
	a space has by summing the '*' 
	characters present in the eight
	adjacent spaces.
*/
int calcNeighbors(int row, int col,int size, char Life[][size])
{
    int neighbors = 0;
 		   if(Life[row - 1][col - 1] == '*')
                               neighbors++;
                        if(Life[row - 1][col] == '*')
                               neighbors++;
                        if(Life[row - 1][col + 1] == '*')
                               neighbors++;
                        if(Life[row][col - 1] == '*')
                               neighbors++;
                        if(Life[row][col + 1] == '*')
                               neighbors++;
                        if(Life[row + 1][col - 1] == '*')
                               neighbors++;
                        if(Life[row + 1][col] == '*')
                               neighbors++;
                        if(Life[row + 1][col + 1] == '*')
                               neighbors++;
    return neighbors;

}

/*
	printBoard
	
	Uses the display.h package in order
	to continuously overwrite the board
	state in the same space. Also displays
	the generation number after each
	successive generation.
*/
void printBoard(int size, int count, char Life[][size])
{
    for(int row =0 ; row<size; row++)
    {
        for(int col = 0; col<size; col++)
        {
            set_cur_pos(row,col);
 	        put(Life[row][col]);	
        }
        puts(" ");
    }
    set_cur_pos(size+2,2);
    char statement[] = "generation: ";
    int lenStr =(int) strlen(statement);
    for(int i = 0; i<lenStr;i++)
    {
 	    set_cur_pos(size+1,i+2);
 	    put(statement[i]);
    }
    set_cur_pos(size+1,lenStr+2);
    printf("%d",count);
    fflush(stdout);
    return;
}
