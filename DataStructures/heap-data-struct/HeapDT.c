#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "HeapDT.h"

#define LEFT(x) 1+2*x
#define RIGHT(x) 2+2*x
#define PARENT(x) (x-1)/2


static int minIndex(Heap aHeap, int index);

struct Heap_S
{
	size_t size;
	size_t capacity;
	void **heap;
	int (*compFun)( const void * lhs, const void * rhs );
	void (*dumpEntry)( const void * item, FILE * outfp );    
};

/// createHeap constructs an empty heap.
/// @return a heap object (pointer)
/// @pre compFun and dumpEntry are not NULL.
/// @post the heap is not NULL (i.e. there was enough storage).
///
Heap createHeap( size_t capacity
               , int (*compFun)( const void * lhs, const void * rhs )
               , void (*dumpEntry)( const void * item, FILE * outfp ) )
{
	Heap h         = malloc( sizeof( struct Heap_S ) );
	h->capacity    = capacity;
	h->size        = 0;
	h->heap        = (void **)calloc( capacity , sizeof(void *) );
	h->compFun     = compFun;
	h->dumpEntry   = dumpEntry;

	return h;
}


/// destroyHeap deletes all dynamic heap storage.
/// @param aHeap the subject heap
/// @pre aHeap is a valid heap data type reference.
/// @pre client previously removed and deleted all entries that
/// involved dynamic memory allocation.
/// @post the aHeap reference is no longer valid.
///
void destroyHeap( Heap aHeap )
{
	free(aHeap->heap);
	free(aHeap);
	return;
}

/// @param aHeap the subject heap
/// @return the current number of active entries in the heap.
/// @pre aHeap is a valid heap data type reference.
///
size_t sizeHeap( Heap aHeap )
{
	return aHeap->size;
}

/// topHeap returns an immutable pointer to the topmost item in the heap.
/// @param aHeap the subject heap
/// @return an immutable pointer to the topmost item.
/// @pre aHeap is a non-empty, valid heap data type instance.
/// @post the internal state of aHeap has not changed.
///
const void * topHeap( const Heap aHeap )
{
	return aHeap->heap[0];
}

/// removeTop removes the topmost item from the heap.
/// @param aHeap the subject heap
/// @return a pointer to the removed item.
/// @pre aHeap is a non-empty, valid heap data type instance.
/// @post client assumes ownership of the item removed.
///
void * removeTopHeap( Heap aHeap )
{
	void * removedHead = aHeap->heap[0];
	for(size_t i = 0; i < aHeap->capacity; ++i)
	{
		if( i == aHeap->capacity - 1 )
		{
			void * temp = aHeap->heap[i];
			aHeap->heap[0]   = temp;
			aHeap->heap[i]   = NULL;
			break;
		}
		else if( aHeap->heap[i] == NULL )
		{
			void * temp = aHeap->heap[i-1];
			aHeap->heap[0]   = temp;
			aHeap->heap[i-1] = NULL;
			break;
		}
	}
	
//----- reduce the tree size --------------------------------------------------
	void **oldHeap = aHeap->heap;
	aHeap->capacity--;
	aHeap->heap = (void **)calloc(aHeap->capacity, sizeof(void *));
	aHeap->size = 0;
	for(size_t i = 0; i < aHeap->capacity; ++i)
	{
	 	if(oldHeap[i])
	 	{
	 		insertHeapItem(aHeap,oldHeap[i]);
	 	}
	}
	free(oldHeap);
// ----------------------------------------------------------------------------
	int currIndex = 0;
	int swapIndex = minIndex(aHeap, currIndex);
	while(swapIndex != currIndex)
	{
			if(swapIndex < 0) break;
			//	Swap
	  		void * tempData = aHeap->heap[currIndex];
	  		aHeap->heap[currIndex] = aHeap->heap[swapIndex];
	  		aHeap->heap[swapIndex] = tempData;
	  		currIndex = swapIndex;
	  		swapIndex = minIndex(aHeap,currIndex);
	}
	return removedHead;
}

/// insertHeapItem inserts an item into the heap.
/// @param aHeap the subject heap
/// @param item the item to insert into the heap
/// @pre aHeap is a valid heap data type instance.
/// @post heap assumes (temporary) ownership/custody of the item, and
/// the client must not delete the memory while heap owns it.
///
void insertHeapItem( Heap aHeap, const void * item )
{	

	// Allocate Memory
	if(aHeap->size == aHeap->capacity)
	{
		void **oldHeap = aHeap->heap;
		aHeap->capacity++;
		aHeap->heap = (void **)calloc(aHeap->capacity, sizeof(void *));
		aHeap->size = 0;
		for(unsigned int i = 0; i < aHeap->capacity-1; ++i)
		{
		 	if(oldHeap[i])
		 	{
		 		insertHeapItem(aHeap,oldHeap[i]);
		 	}
		}
		free(oldHeap);
	}

	// Insert
	int index = 0;
	while(aHeap->heap[index])
		index++;
	aHeap->heap[index] = (void *) item;

	// Sift Up
	while(index > 0 && !aHeap->compFun( aHeap->heap[PARENT(index)], aHeap->heap[index]) )
	{
	  void * tempData = aHeap->heap[index];
	  aHeap->heap[index] = aHeap->heap[PARENT(index)];
	  aHeap->heap[PARENT(index)] = tempData;
	  index = PARENT(index);
	}
	aHeap->size++;
	return;
}

/// minNode returns the index of the smallest of a parent index and its children
/// @param aHeap the subject heap
/// @param index the parent index
/// @pre aHeap is a valid heap data type instance
/// @return the index of the smallest
/// @post the internal state has not changed
static int minIndex(Heap aHeap, int index)
{
	void *par,*lt,*rt;
	if( (size_t)index < aHeap->size )
		par = aHeap->heap[index];
	else par = NULL;

	if( (size_t)LEFT(index) < aHeap->size )
		lt  = aHeap->heap[LEFT(index)];
	else lt = NULL;

	if( (size_t)RIGHT(index) < aHeap->size )
		rt  = aHeap->heap[RIGHT(index)];
	else rt = NULL;

	if(!par) return -1;

    if(rt && lt)
    {
		if(aHeap->compFun(lt,par) &&aHeap->compFun(lt,rt))
			return LEFT(index);
		else if(aHeap->compFun(rt,par) && aHeap->compFun(rt,par))
			return RIGHT(index);
		else
			return index;
	}
	else if(rt)
	{
		if(aHeap->compFun(rt,par)) return RIGHT(index);
		else return index;
	}
	else if(lt)
	{
		if(aHeap->compFun(lt,par)) return LEFT(index);
		else return index;
	}
	else return index;
}

/// dumpHeap prints the heap content to the specified output file stream.
/// @param aHeap the subject heap
/// @param outfp the output file stream
/// @pre aHeap is a valid heap data type instance.
/// @post the internal state of aHeap has not changed.
///
void dumpHeap( Heap aHeap, FILE * outfp )
{
	for(size_t i = 0; i < aHeap->capacity; ++i)
	{
		if( aHeap->heap[i] != NULL )
		{
			aHeap->dumpEntry(aHeap->heap[i], outfp);
		}
	}
	return;
}

