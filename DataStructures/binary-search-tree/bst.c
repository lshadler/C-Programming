#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include "bst.h"
#include <string.h>
#include <ctype.h>

/*
// Main Program
//
// Takes input from the command line
// and converts it to individual strings.
// These word strings are then inserted 
// into the tree. after the tree is 
// produced, it will be traversed in order
// to print word and frequency. the tree
// pointers are then freed.
*/
int main(int argc,char ** argv)
{
   if(argc != 1)
   {
	printf("Usage: %s\n",argv[0]);
	return 1;
   }
   TreeNode*  treeRoot = NULL;
   char *buf = NULL;
   size_t len = 0;
   //Whatsup
   while( getline(&buf,&len,stdin) > 0 )
   {
       
       const char * str;
       str = strtok (buf," ,!?.*");
       while (str != NULL)
       {
	   if(strlen(str) ==1 && isspace(str[0])){}
	   else 
	   	insert_word(&treeRoot,str);
           str = strtok (NULL, " ,!?.*");
       }

   }

   if(buf != NULL)
   {
       free(buf);
   }
   
   traverse_tree( treeRoot );
   cleanup_tree ( treeRoot );

   return 0;
}

/*
// insert_word
//
// enters tree, looking for the first pointer that
// matches the characteristics of the argument
// word. If it dosen't exist in this setup yet,
// meaning this location contains a null pointer,
// then put the word in this location.
*/
void insert_word( TreeNode** root, const char *word )
{
   if(*root == NULL)
   {

	*root  = (TreeNode*) malloc( sizeof( TreeNode ) );
	(*root)->word = strdup(word);

	//strcpy((*root)->word , word);

	(*root) -> frequency = 1;
	(*root) -> left  = NULL;
	(*root) -> right = NULL;
	return;
   }
   
	int test = strcasecmp( (*root)->word, word );
	if(test == 0)
	{
	   int numFreqRoot = (*root)->frequency;
	   numFreqRoot++;
	   (*root)->frequency = numFreqRoot;
	}	
	else if(test>0) insert_word( &((*root)->left), word);
	else insert_word( &((*root)->right),word);
   return;
}

/*
// traverse_tree
//
// travels across the tree, printing the words 
// and frequency in alphabetical order.
//
*/
void traverse_tree( const TreeNode* root )
{
   if(root != 0)
   {
     traverse_tree( root->left );
     printf("%s %d\n",root->word , root->frequency );
     traverse_tree( root->right );
   }
   return;
}

/*
// cleanup_tree
//
// deallocates memory used up by the binary
// tree
// 
*/
void cleanup_tree( TreeNode* root )
{
   if(root == NULL) return; 
   
      cleanup_tree(root ->left  );
      cleanup_tree(root ->right );
      free(root->word);
      free(root);
   
}
