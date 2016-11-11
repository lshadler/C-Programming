/**
 *  AmigoNet.c
 *  Luke Shadler
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "table.h"
#include "amigonet.h"
#include "hash.h"
#include "amigomem.h"
/**
 * declaration of the user structure
 */
typedef struct User_S
{
 struct Key_S *id;
 size_t numFriends;
 struct Amigo_S *friends;
}User;

typedef struct Amigo_S
{
 struct User_S * data;
 struct Amigo_S *next;
}Amigo;

typedef struct Key_S
{
 char * name;
 char *birthday;
}Key;

struct Key_S* makeKey(const char* name, const char* birthdate);
bool userEquals(void* key1, void* key2);
void userPrint(void * key, void * value);
long userHash(void * key);

static Table * UserTable = NULL;

/**
 * Initialize the system.
 * (This is where some memory is initially allocated.)
 */
void create_amigonet()
{
  UserTable = create(userHash,userEquals,userPrint);
}



/**
 * Shut down the system.
 * (This is where a bunch of memory is freed.)
 */
void destroy_amigonet()
{
  for(unsigned int i = 0; i<UserTable->capacity;++i)
  {
   if(UserTable->table[i])
   {

    free( ((Key*)UserTable->table[i]->key)->name);
    free( ((Key*)UserTable->table[i]->key)->birthday);
    free(UserTable->table[i]->key);
     Amigo* temp = (((User*)UserTable->table[i]->value)->friends);
Amigo* temp1 = NULL;
while(temp != NULL ){
	temp1 = temp->next;
	free(temp);
	temp = temp1;
}

    free(UserTable->table[i]->value);
   }
  }
  destroy(UserTable);
}


/**
 * Add a new user with initially no friends.
 * The parjameters 'name' and 'birthday' are used to initialize the new user entry.
 * Note: they must be copied to prevent the caller from changing
 * the them later.
 * If the name already exists, then this function does nothing, except for
 * print an error message.
 */
void addUser( const char *name, const char *birthdate )
{
 char year[5];
 strncpy(year,birthdate+4,4);
 year[4] = '\0';
 if(strlen(name) > MAX_NAME_LENGTH)
 {
   fprintf(stderr,"Error, invalid arguments for addUser");
   assert(NULL);
 }
 if((long)year < 1700 && (long)year > 2200)
 {
  fprintf(stderr,"Error, invalid arguments for addUser");
  assert(NULL);
 }
 
 else
 {
  User *temp = calloc(1,sizeof(User));
  temp->id = makeKey(name,birthdate);
  temp->numFriends=0;
  temp->friends = NULL;  
  put(UserTable,temp->id,temp);
 }
}


/**
 * Print out the specified user's friends in the order that they were
 * "friended"
 */
void printAmigos( User *user )
{
 if(user->numFriends == 1)
  printf("%lu friend\n",user->numFriends);
 else
  printf("%lu friends\n",user->numFriends);
 Amigo* friend = user-> friends;
 while(friend)
 {
  UserTable->print(friend->data->id,friend->data);
  friend = friend->next;
  printf("\n");
 }
}


/**
 * Print the number of users.
 */
void printNumUsers(void)
{
  printf("Registered Users: %lu\n",UserTable->size);
}


/**
 * Locate a user structure using the user's name and birthdate as a key.
 * User structures are needed for the addAmigo, removeAmigo,
 * and printFriends functions.
 * If the user does not exist, NULL is returned.
 *
 */
User *findUser( const char *name, const char *birthdate )
{
 Key * key = makeKey(name,birthdate);
  User* temp = NULL; 
 if(!has(UserTable,key))
 {
  //fprintf(stdout,"Error, user doesn't exist\n");
 }
 else
 {
  temp = get(UserTable,key);
 }
  free(key->name);
  free(key->birthday);
  free(key);
  return temp;

}


/**
 * Add a friend (the "amigo") to the user. This should be a two-way
 * addition. If the two users are already friends, this function
 * does nothing except print an error message. 
 */
void addAmigo( User *user, User *amigo )
{
 if(user == NULL || amigo == NULL)
 {
  fprintf(stderr,"Error, user doesn't exist");
  return;
 }



 if(UserTable->equals(user->id,amigo->id))
 {
  fprintf(stderr,"Error, cannot friend yourself.\n");
  return;
 }

 Amigo* tempAmigo = malloc(sizeof(Amigo));
 tempAmigo->data = amigo;
 tempAmigo->next = NULL;

 if(user->friends == NULL)
 {
   user->friends = tempAmigo;
   user->numFriends++;
 }
 else
 {
   Amigo* curr = user->friends;
   while(curr->next)
   {
     if(curr->data && UserTable->equals(curr->data->id,amigo->id))
     {
      fprintf(stderr,"Error, %s and %s are already friends.\n",user->id->name,amigo->id->name);
      free(tempAmigo);
      return;
     }
     curr = curr->next;
   }
   if(curr->data && UserTable->equals(curr->data->id,amigo->id))
   {
    fprintf(stderr,"Error, %s and %s are already friends.\n",user->id->name,amigo->id->name);
    free(tempAmigo);
    return;
   }
   user->numFriends++;
   curr->next = tempAmigo;
 }



 Amigo* tempUser = malloc(sizeof(Amigo));
 tempUser->data = user;
 tempUser->next = NULL;

 if(amigo->friends == NULL)
 {
   amigo->numFriends++;
   amigo->friends = tempUser;
 }
 else
 {
   Amigo* curre = amigo->friends;
   while( curre->next)
   {
     if(curre->data && UserTable->equals(curre->data->id,user->id))
     {
      fprintf(stderr,"Error, %s and %s are already friends.\n",amigo->id->name,user->id->name);
      free(tempUser);
      return;
     }
     curre = curre->next;
   }
   if(UserTable->equals(curre->data->id,user->id))
   {
    fprintf(stderr,"Error, %s and %s are already friends.\n",amigo->id->name,user->id->name);
    free(tempUser);
    return;
   }
   amigo->numFriends++;
   curre->next = tempUser;
 }
}




/**
 * "Un-friend" two users. This is, again, a two-way operation.
 * If the two users were not friends, this function does nothing,
 * except print an error message.
 */
void removeAmigo(User *user,  User *ex_amigo )
{
 if(user == NULL || ex_amigo == NULL)
 {
  fprintf(stderr,"Error, user doesn't exist");
  return;
 }
 if(UserTable->equals(user->friends->data->id,ex_amigo->id))
 {
  user->numFriends--;
  if(user->friends->next)
   user->friends = user->friends->next;
  else
   free(user->friends);
 }
 else
 {
   Amigo* curr = user->friends;
   while(curr->next)
   {
    if(UserTable->equals(curr->next->data->id,ex_amigo->id))
    {
     user->numFriends--;
     if(curr->next->next)
     {
       curr->next = curr->next->next;
     }
     else
     {
      free(curr->next);
     }
    }
   }
   fprintf(stderr,"Error, %s and %s are not friends.\n",user->id->name,ex_amigo->id->name);
 }
 

 
 if(UserTable->equals(ex_amigo->friends->data->id,user->id))
 {
  ex_amigo->numFriends--;
  if(ex_amigo->friends->next)
   ex_amigo->friends = ex_amigo->friends->next;
  else
   free(ex_amigo->friends);
 }
 else
 {
   Amigo* curr = user->friends;
   while(curr->next)
   {
    if(UserTable->equals(curr->next->data->id,user->id))
    {
     ex_amigo->numFriends--;
     if(curr->next->next)
     {
       curr->next = curr->next->next;
     }
     else
     {
      free(curr->next);
     }
    }
   }
 }
 
}

Key* makeKey(const char *name, const char *birthdate)
{
  Key* key = calloc(1,sizeof(Key));
  key->name = calloc(strlen(name)+1,sizeof(char));
  strncpy(key->name,name,strlen(name));
  key->birthday = calloc(strlen(birthdate)+1,sizeof(char)); 
  strncpy(key->birthday, birthdate, strlen(birthdate));
  return key;
}

void userPrint(void * key, void * value)
{
 printf("%s, %s",((Key*)key)->name,((User*)value)->id->birthday); 
}

long userHash(void * key)
{
 long hashed = 0;
 hashed += strHash((void*)((Key*)key)->name);
 hashed += strHash((void*)((Key*)key)->birthday);
 
 return hashed;
}

bool userEquals(void * key1, void * key2)
{
  int cmpName  = strcmp(((Key*)key1)->name,((Key*)key2)->name);
  int cmpBirth = strcmp(((Key*)key1)->birthday,((Key*)key2)->birthday);
  if(cmpName == 0 && cmpBirth == 0)
    return true;
  else
    return false;
}
