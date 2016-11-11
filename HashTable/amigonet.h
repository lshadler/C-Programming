/**
 * amigonet.h
 * $Revision: 1.10 $
 * James Heliotis
 * Updated by: Chris Dickens
 *
 * AmigoNet
 * A CSci 243 project that focuses on memory allocation and data structures
 *
 * The idea is a simulation of a social network. New users can register,
 * and friends ("amigos") can be added or removed.
 */

#ifndef AMIGONET_H
#define AMIGONET_H


#define MAX_NAME_LENGTH 39

/**
 * Forward declaration of the user structure
 */
struct User_S;


/**
 * Initialize the system.
 * (This is where some memory is initially allocated.)
 */
void create_amigonet();


/**
 * Shut down the system.
 * (This is where a bunch of memory is freed.)
 */
void destroy_amigonet();


/**
 * Add a new user with initially no friends.
 * The parameters 'name' and 'birthday' are used to initialize the new user entry.
 * Note: they must be copied to prevent the caller from changing
 * the them later.
 * If the name already exists, then this function does nothing, except for
 * print an error message.
 */
void addUser( const char *name, const char *birthdate );


/**
 * Print out the specified user's friends in the order that they were
 * "friended"
 */
void printAmigos( struct User_S *user );


/**
 * Print the number of users.
 */
void printNumUsers(void);


/**
 * Locate a user structure using the user's name and birthdate as a key.
 * User structures are needed for the addAmigo, removeAmigo,
 * and printFriends functions.
 * If the user does not exist, NULL is returned.
 */
struct User_S *findUser( const char *name, const char *birthdate );


/**
 * Add a friend (the "amigo") to the user. This should be a two-way
 * addition. If the two users are already friends, this function
 * does nothing except print an error message. 
 */
void addAmigo( struct User_S *user, struct User_S *amigo );


/**
 * "Un-friend" two users. This is, again, a two-way operation.
 * If the two users were not friends, this function does nothing,
 * except print an error message.
 */
void removeAmigo( struct User_S *user, struct User_S *ex_amigo );

#endif
