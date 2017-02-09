// File: $Id: bst.h,v 1.1 2015/10/20 19:27:16 lxs2208 Exp $
//
// Binary Search Tree definitions
//
// Author: rwd, sps, wrc

#ifndef _BST_H_
#define _BST_H_

/// The definition of the tree structure
typedef struct TreeNode_st {
    char *word;                   // the word held in this node
    unsigned int frequency;       // how many times it has been seen
    struct TreeNode_st *left;     // node's left child
    struct TreeNode_st *right;    // node's right child
} TreeNode;

// FUNCTIONS STUDENTS ARE REQUIRED TO IMPLEMENT

/// insert_word() 
///     Dynamically build BST by allocating nodes from the heap
///
/// args -
///        root - a pointer to the pointer to the root of the tree
///               to build this tree on to.
///        word - string containing the word to be inserted

void insert_word( TreeNode** root, const char *word );

/// traverse_tree()
///    Recursively traverses the tree and prints the value of each
///    node.
///
/// args -
///        root - a pointer to the root of the tree to traverse

void traverse_tree( const TreeNode* root );

/// cleanup_tree()
///    Cleanup all memory management associated with the nodes on the heap
///
/// args
///      root - the current root of the tree

void cleanup_tree( TreeNode* root );

#endif // BST_H

// Revisions: $Log: bst.h,v $
// Revisions: Revision 1.1  2015/10/20 19:27:16  lxs2208
// Revisions: Files have been added. Tree works properly but the memory management is problematic
// Revisions:
// Revisions: Revision 1.3  2015/10/15 01:44:53  csci243
// Revisions: support for trees containing strings
// Revisions:
// Revisions: Revision 1.2  2013/10/03 12:49:03  csci243
// Revisions: added docstrings
// Revisions:
// Revisions: Revision 1.1  2013/10/03 02:06:55  csci243
// Revisions: Initial revision
// Revisions:
