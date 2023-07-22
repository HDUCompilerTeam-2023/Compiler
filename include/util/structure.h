#ifndef __UTIL_STRUCTURE__
#define __UTIL_STRUCTURE__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    int stacktop;
    int stacksize;

    uint64_t *stackdata;
} stack;

stack *InitStack();
void stack_push(stack *pushstack, uint64_t pushnumber);
uint64_t stack_pop(stack *popstack);
uint64_t stack_top(stack *popstack);
bool checkstack(stack *checkstack);
void destroystack(stack *stk);
typedef enum Color {
    RED,
    BLACK
} Color;

typedef struct Node {
    uint64_t key;
    Color color;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
} Node;

typedef struct RedBlackTree {
    Node *root;
} RedBlackTree;

Node *createNode(uint64_t key, Color color);
RedBlackTree *initializeRedBlackTree();
void leftRotate(RedBlackTree *tree, Node *x);
void rightRotate(RedBlackTree *tree, Node *x);
void insertFixup(RedBlackTree *tree, Node *z);
bool insert(RedBlackTree *tree, uint64_t key);
bool search(Node *root, uint64_t key);
void freeNode(Node *node);
void clearRedBlackTreeNode(Node *root);
void clearRedBlackTree(RedBlackTree *tree);
void destroyRedBlackTree(RedBlackTree *tree);

#endif