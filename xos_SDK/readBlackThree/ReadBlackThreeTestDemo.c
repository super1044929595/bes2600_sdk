#include <stdio.h>
#include <stdlib.h>
#include "RedBlackTree.h"

/**********************红黑树使用示例 start***************************/
// 遍历
void rbtree_traversal(RedBlackTree *T, rbtree_node *node) {
	if (node != T->nil) {
		rbtree_traversal(T, node->left);
		printf("key:%d, color:%d\n", node->key, node->color);
		rbtree_traversal(T, node->right);
	}
}

int main() {

	int keyArray[20] = { 24,25,13,35,23, 26,67,47,38,98, 20,19,17,49,12, 21,9,18,14,15 };

	RedBlackTree *T = (RedBlackTree *)malloc(sizeof(RedBlackTree));
	if (T == NULL) {
		printf("malloc failed\n");
		return -1;
	}

	T->nil = (rbtree_node*)malloc(sizeof(rbtree_node));
	T->nil->color = BLACK;
	T->root = T->nil;

	rbtree_node *node = T->nil;
	int i = 0;
	for (i = 0; i < 20; i++) {
		node = (rbtree_node*)malloc(sizeof(rbtree_node));
		node->key = keyArray[i];
		node->value = NULL;
		printf("insert arr[%d]=%d\n",i,keyArray[i]);
		rbtree_insert(T, node);
		printf("insert end\n");

	}
	printf("----------------------------------------\n");
	rbtree_traversal(T, T->root);
	printf("----------------------------------------\n");

	for (i = 0; i < 20; i++) {
		printf("search key = %d\n", keyArray[i]);
		rbtree_node *node = rbtree_search(T, keyArray[i]);

		if(node!=T->nil)
		{
			printf("delete key = %d\n", node->key);
			rbtree_node *cur = rbtree_delete(T, node);
			free(cur);
		}
		else
			break;
		
		printf("show rbtree: \n");
		rbtree_traversal(T, T->root);
		printf("----------------------------------------\n");
	}
    if(T!=NULL)
        free (T);
}

// gcc -o example main.c ../src/RedBlackTree.c
/**********************红黑树使用示例 end***************************/