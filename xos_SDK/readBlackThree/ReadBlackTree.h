#ifndef _FLY_RED_BLACK_TREE_H_
#define _FLY_RED_BLACK_TREE_H_

#define KEY_TYPE    int

enum RB_COLOR{RED=0,BLACK};

#if 0
#define RBTREE_ENTRY(name,type)\
    struct name                         \
    {                                   \
        struct _rbtree_node*    left;   \
        struct _rbtree_node*    right;  \
        struct _rbtree_node*    parent; \
        unsigned char           color;  \
    }
    
#endif

typedef struct _rbtree_node
{
    /* data */
    KEY_TYPE                key;
    void*                   value;
    
    struct _rbtree_node*    left;   
    struct _rbtree_node*    right;  
    struct _rbtree_node*    parent; 
    unsigned char           color;  
#if 0
    RBTREE_ENTRY(,_rbtree_node) node;
    // RBTREE_ENTRY(,_rbtree_node) node2;
    // RBTREE_ENTRY(,_rbtree_node) node3;
#endif
    
}rbtree_node;

typedef struct _RedBlackTree
{
    /* data */
    rbtree_node *root;
    rbtree_node *nil;

}RedBlackTree;

extern void rbtree_insert(RedBlackTree *T,rbtree_node *z);
extern rbtree_node *rbtree_delete(RedBlackTree *T, rbtree_node *z) ;
extern rbtree_node *rbtree_search(RedBlackTree *T, KEY_TYPE key);

#endif