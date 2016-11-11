/*
 * amigosim.c
 *
 * Simulate AmigoNet activity by reading actions from a text file.
 *
 * James Heliotis, Chris Dickens
 *
 * COMMAND FORMATS:
 * register <name> <birthdate>
 * friend <name1> <birthdate1> <name2> <birthdate2>
 * unfriend <name1> <birthdate1> <name2> <birthdate2>
 * printFriends <name> <birthdate>
 * countUsers
 *
 * Comment lines begin with "#".
 * They are echoed in the output, but otherwise ignored.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "table.h"
#include "amigonet.h"

//#ifndef UNINSTRUMENTED
#include "amigomem.h"
//#endif


//#define DEBUG


static const char REGISTER[] = "addUser";
static const char FRIEND[] = "friend";
static const char UNFRIEND[] = "unfriend";
static const char PRINTFRIENDS[] = "printFriends";
static const char PRINTNUMUSERS[] = "countUsers";
static const char EXIT[] = "exit";

#define MAX_LEN 100 // This big for comments


/*************************************************************
 *                                                           *
 * Calls to the AmigoNet API                                 *
 *                                                           *
 *************************************************************/

static void do_register( const char *name, const char *birthdate )
{
    addUser( name, birthdate );
}


static void do_printFriends( const char *name, const char *birthdate )
{
  printAmigos(findUser(name,birthdate));
}

static void do_printNumUsers(void)
{
    printNumUsers();
}

static void do_friend( const char *name1, const char *birthdate1, 
    const char *name2, const char *birthdate2 )
{
 if(findUser(name1,birthdate1) && findUser(name2,birthdate2))
  addAmigo(findUser(name1,birthdate1),findUser(name2,birthdate2));
}

static void do_unfriend( const char *name1, const char *birthdate1,
    const char *name2, const char *birthdate2 )
{
 if(findUser(name1,birthdate1) && findUser(name2,birthdate2))
  removeAmigo(findUser(name1,birthdate1),findUser(name2,birthdate2));
}


/*************************************************************
 *                                                           *
 * Helper Functions                                          *
 *                                                           *
 *************************************************************/
static char* alloc_and_cat(char* str1, char* str2)
{
    char* ptr;
    int size = strlen(str1) + strlen(str2) + 2;

    ptr = (char*)malloc(size);
    if(ptr == NULL)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        assert(NULL);
    }

    strncpy(ptr, str1, size);
    strncat(ptr, " ", size-strlen(str1));
    strncat(ptr, str2, size-strlen(str1)-1);
   
    return ptr;
}


/*************************************************************
 *                                                           *
 * Command input loop                                        *
 *                                                           *
 *************************************************************/

int main() {
    create_amigonet();
    char line[ MAX_LEN ];
    char *result, *cmd, *arg1, *arg2,*arg1a,*arg2a;
    char *name1, *name2, *birthdate1, *birthdate2;
    printf("> ");
    fflush(stdout);
    while ( ( result = fgets( line, MAX_LEN, stdin ) ) ) {
        if ( line[ 0 ] == '#' ) {
            printf( "%s", line );
        }
        else {
            #ifdef DEBUG
            printf("%s", line);
            #endif
            cmd = strtok( line, " \n" );
            if( cmd == NULL)
            {
                // do nothing
            }
            else if ( strcmp( cmd, REGISTER ) == 0 )
            {
                arg1 = strtok( NULL, " \n" );
                arg2 = strtok( NULL, " \n" );
                birthdate1 = strtok( NULL, " \n" );
                if( arg1 && arg2 && birthdate1 && (birthdate1[2] == '/') && (birthdate1[5] == '/'))
                {
                    name1 = alloc_and_cat( arg1, arg2 );
                    do_register( name1, birthdate1 );
                    free(name1);
                }
                else
                {
                    fprintf(stderr, "Error, invalid arguments for addUser\n");
                }
            }
            else if ( strcmp( cmd, PRINTFRIENDS) == 0 )
            {              
                arg1 = strtok( NULL, " \n" );
                arg2 = strtok( NULL, " \n" );
                birthdate1 = strtok( NULL, " \n" );
                if( arg1 && arg2 && birthdate1 && (birthdate1[2] == '/') && (birthdate1[5] == '/'))
                {
                    name1 = alloc_and_cat( arg1, arg2 );
                    do_printFriends( name1, birthdate1 );
                    free(name1);
                }
                else
                {
                    fprintf(stderr, "Error, invalid arguments for printFriends\n");
                }
            }
            else if ( strcmp( cmd, PRINTNUMUSERS) == 0 )
            {
                
                do_printNumUsers();
            }
            else if ( strcmp( cmd, EXIT) == 0 )
            {
                break;
            }
            else if ( strcmp( cmd, FRIEND ) == 0 ) 
            {

                arg1  = strtok( NULL, " \n" );
                arg1a = strtok( NULL, " \n" );
                birthdate1 = strtok( NULL, " \n" );
                arg2  = strtok( NULL, " \n" );
                arg2a = strtok( NULL, " \n" );
                birthdate2 = strtok( NULL, " \n" );
                if( arg1 && arg2 && birthdate1 && birthdate2 && (birthdate1[2] == '/') && (birthdate1[5] == '/') && (birthdate2[2]=='/') && (birthdate2[5]=='/'))
                {
                    name1 = alloc_and_cat( arg1, arg1a );
                    name2 = alloc_and_cat( arg2, arg2a );
                    do_friend( name1, birthdate1,name2 ,birthdate2 );
                    free(name1);
                    free(name2);
                }
                else
                {
                    fprintf(stderr, "Error, invalid arguments for Friend\n");
                }
            }
            else if ( strcmp( cmd, UNFRIEND ) == 0 )
            {
                arg1  = strtok( NULL, " \n" );
                arg1a = strtok( NULL, " \n" );
                birthdate1 = strtok( NULL, " \n" );
                arg2  = strtok( NULL, " \n" );
                arg2a = strtok( NULL, " \n" );
                birthdate2 = strtok( NULL, " \n" );
                if( arg1 && arg2 && birthdate1 && birthdate2 && (birthdate1[2] == '/') && (birthdate1[5] == '/') && (birthdate2[2]=='/') && (birthdate2[5]=='/'))
                {
                    name1 = alloc_and_cat( arg1, arg1a );
                    name2 = alloc_and_cat( arg2, arg2a );
                    do_unfriend( name1, birthdate1,name2 ,birthdate2 );
                    free(name1);
                    free(name2);
                }
                else
                {
                    fprintf(stderr, "Error, invalid arguments for Unfriend\n");
                }
            }
            else {
                fprintf( stderr, "Error, illegal command \"%s\"\n", line );
            }
        }
        printf("> ");
        fflush(stdout);
    }
    destroy_amigonet();
}

