// \file table.c
// \brief a generic hash table data structure
// Author: Lucas Shadler

#include <time.h>
#include <string.h>
#include "table.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "hash.h"




/*
// Table * create
//
// Creates a new table structure given
// the predetermined hash, equals, and
// print functions.
//
*/


Table* create(long (*hash)(void* key), bool (*equals)(void* key1, void* key2),
	      void (*print)(void* key1, void* key2))
{
 Table* temp = (Table*) malloc( sizeof( Table ) );
 if (temp == NULL)
 {
  fprintf(stderr, "table::create() failed to allocate space");
  assert(NULL);
 }
 temp->hash       = hash;
 temp->equals     = equals;
 temp->print      = print;
 temp->size       = 0;
 temp->capacity   = INITIAL_CAPACITY;
 temp->collisions = 0;
 temp->rehashes   = 0;
 temp->table      = calloc(INITIAL_CAPACITY, sizeof(Entry) ); 
 if(temp->table == NULL)
 {
  fprintf(stderr, "table::create() failed to allocate tablespace");
  assert(NULL);
 }
 
 return temp;
}


/*
//
// void destroy(Table t)
//
// free the allocated memory produced
// in the table. 
//
*/


void destroy(Table* t)
{
 for(unsigned int i = 0; i< t->capacity ; ++i)
 {
   free(t->table[i]);
 }
 free(t->table);
 free(t);
 return;
}


/*
//
// void dump
//
// prints the size, capacity, and
// number of collisions and rehashes
// 
// if full == true, the entire table
// will be dumped with key/value pairs
// defined by the print function.
//
*/


void dump(Table* t, bool full)
{
 printf("Size: %lu",t->size);
 printf("\nCapacity: %lu",t->capacity);
 printf("\nCollisions: %lu",t->collisions);
 printf("\nRehashes: %lu\n",t->rehashes);

 if(full)
 {
  for(unsigned int i = 0; i< t->capacity; i++)
  {
   printf("%d: ",i);
   if(t->table[i] != NULL && t->table[i]->key != NULL && t->table[i]->value != NULL)
   {
    printf("(");
    t->print(t->table[i]->key,t->table[i]->value);
    printf(")");
   }
   else
   {
    printf("null");
   }
  printf("\n");
  }
 }

 return;
}


/*
// void* get
// 
// gives the value associated with the given 
// key, if the key is within the table.
*/


void* get(Table* t, void* key)
{

//  if(!has(t,key))
//  {
//   fprintf(stderr,"table:get() : The table does not contain this key\n");
//   assert(NULL);
//  }
 
 unsigned int index = (unsigned int) t->hash(key) % t->capacity;
 while(t->table[index] != NULL && !(t->equals(t->table[index]->key,key)))
 {
  index= (index+1) % t->capacity;

  t->collisions++;
  if(index == t->hash(key) % t->capacity)
  {
   fprintf(stderr,"table:get() : didn't find the key\n");
   assert(NULL);
  }
 }
 if(t->table[index]==NULL)
 {
  fprintf(stderr, "table:get() : didn't find the key\n");
  assert(NULL);
 }
 return t->table[index]->value;

}


/*
// bool has
//
// checks to see if the table contains
// the given key
*/


bool has(Table* t, void* key)
{
  unsigned int index =(unsigned int) t->hash(key) % t->capacity;
  
 while(t->table[index] != NULL && !(t->equals(t->table[index]->key,key)))
 {
  index= (index+1) % t->capacity;

  t->collisions++;
  if(index == t->hash(key) % t->capacity)
  {
   fprintf(stderr,"table:has() : There is no valid location for key\n");
   assert(NULL);
  }
 }
 
 if(t->table[index] == NULL) return false;
 else return true;
}


/*
// void ** keys
//
// returns the unordered key set of the given table
*/


void** keys(Table* t)
{
 void ** keySet = (void**) malloc(t->size * sizeof(void*));
 long thisSize = t->size;
 int j = 0;
 for(unsigned int i = 0; i< t->capacity;i++)
 {
  if(t->table[i] && t->table[i]->key != NULL)
  {
   keySet[j] = t->table[i]->key;
   j++;
   thisSize--;
  }
  if(thisSize == 0) break;
 } 
 return keySet;
}


/*
// void * put
//
// puts a new Entry into the table
// if the key is already in, overwrite the value
// if the LOAD_THRESHOLD is reached, then rehashes
//   the table.
*/


void* put(Table* t, void* key, void* value)
{
  
//----------Rehash Function----------------
  
  if(t->size >= LOAD_THRESHOLD* t->capacity)
  {
   t->rehashes++;
   Entry** oldTable = t->table;
   unsigned int cap = t->capacity;
   t->capacity = RESIZE_FACTOR * (t->capacity);
   t->table   = (Entry**) calloc(t->capacity, sizeof(Entry*));
   if(t->table == NULL)
   {
    fprintf(stderr,"table:put() : failed to allocate table");
    assert(NULL);
   }
   t->size = 0;
   for(unsigned int ind = 0; ind<cap; ++ind)
   {
    if(oldTable[ind])
    {
     put(t, oldTable[ind]->key,oldTable[ind]->value);
     free(oldTable[ind]);
    }
   }
   free(oldTable);
   
  }

//------------------------------------------- 

 unsigned int index =(unsigned int) t->hash(key) % t->capacity;
 void * oldValue=NULL;



 while(t->table[index] != NULL && !(t->equals(t->table[index]->key,key)))
 {
  index= (index+1) % t->capacity;
  t->collisions++;
  if(index == t->hash(key) % t->capacity)
  {
   fprintf(stderr,"table:put() : There is no valid location for key");
   assert(NULL);
  }
 }
  if(t->table[index]==NULL)
  {
   t->table[index] = (Entry*) malloc( sizeof(Entry) );
   t->table[index]->key = key;
   t->table[index]->value = value;
   t->size++;
   return oldValue;
  }
  else
  {
    oldValue = t->table[index]->value;
    free(t->table[index]);
    t->table[index] = (Entry*) malloc(sizeof(Entry));
    t->table[index]->key   = key;
    t->table[index]->value = value;
    return oldValue;
  }

 fprintf(stderr,"table:put() : put failed...");
 assert(NULL);
}


/*
// void** values
//
// returns all of the values contained within the table
*/


void** values(Table* t)
{
 void ** vals = (void**) malloc(t->size * sizeof(void*));
 unsigned int numVals=0;
 for(unsigned int i = 0; i < t->capacity; ++i)
 {
  if(t->table[i] != NULL && t->table[i]->key != NULL)
  {
   vals[numVals] = t->table[i]->value;
   numVals++;
  }
  if(numVals == t->size) break;
 }
 return vals;
}

/*-------------------------------------
|            END OF TABLE.C           |
---------------------------------------*/
