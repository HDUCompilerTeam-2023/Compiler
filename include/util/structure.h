#ifndef __UTIL_STRUCTURE__
#define __UTIL_STRUCTURE__

#include <stdbool.h>
#include <stdio.h>

typedef enum Color {
    RED,
    BLACK
} Color;

typedef struct Node {
    size_t key;
    Color color;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
} Node;

typedef struct RedBlackTree {
    Node *root;
} RedBlackTree;

Node *createNode(size_t key, Color color);
RedBlackTree *initializeRedBlackTree();
void leftRotate(RedBlackTree *tree, Node *x);
void rightRotate(RedBlackTree *tree, Node *x);
void insertFixup(RedBlackTree *tree, Node *z);
bool insert(RedBlackTree *tree, size_t key);
bool search(Node *root, size_t key);
void freeNode(Node *node);
void clearRedBlackTreeNode(Node *root);
void clearRedBlackTree(RedBlackTree *tree);
void destroyRedBlackTree(RedBlackTree *tree);

#endif