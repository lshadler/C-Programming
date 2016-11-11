/// \file amigomem.h
/// \brief Include this file to use libamigomem to track memory usage.
/// Author: Chris Dickens (RIT CS)

#include <stdlib.h>

// Use the AmigoNet_malloc wrapper
#define malloc AmigoNet_malloc

// Use the AmigoNet_free wrapper
#define free AmigoNet_free

// Use the AmigoNet_realloc wrapper
#define realloc AmigoNet_realloc

// Use the AmigoNet_calloc wrapper
#define calloc AmigoNet_calloc

// Wrapper function to track malloc allocations
void* AmigoNet_malloc(size_t size);

// Wrapper function to track deallocations
void AmigoNet_free(void* addr);

// Wrapper function to track allocations and frees
// that occur using realloc
void* AmigoNet_realloc(void *ptr, size_t size);

// Wrapper function to track allocations that occur
// using calloc
void* AmigoNet_calloc(size_t num, size_t size);

