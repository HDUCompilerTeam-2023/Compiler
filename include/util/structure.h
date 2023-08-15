#ifndef __UTIL_STRUCTURE__
#define __UTIL_STRUCTURE__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct HashNode {
    uint64_t key;
    uint64_t val;
    struct HashNode *next;
} HashNode;

typedef struct HashTable {
    HashNode **table;
    unsigned int size;
} HashTable;

void destroyHashTable(HashTable *ht);
void hashremoveNode(HashTable *ht, uint64_t key);
uint64_t hashfind(HashTable *ht, uint64_t key);
void hashinsert(HashTable *ht, uint64_t key, uint64_t val);
HashTable *initHashTable(uint32_t size);
unsigned int hash(uint64_t key, unsigned int size);

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

typedef struct {
    uint64_t addr;
    int goal;
} heap_node;

typedef struct {
    heap_node *arr;
    int size;
    int capacity;
} MaxHeap;

MaxHeap *createMaxHeap(int capacity);
void destroyMaxHeap(MaxHeap *heap);
void heapify(MaxHeap *heap, int idx);
void heap_push(MaxHeap *heap, heap_node value);
uint64_t heap_pop(MaxHeap *heap);
uint64_t heap_top(MaxHeap *heap);
void destroyMaxHeap(MaxHeap *heap);

#endif