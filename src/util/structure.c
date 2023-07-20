#include <stdio.h>
#include <stdlib.h>
#include <util/structure.h>

// tree

Node *createNode(size_t key, Color color) {
    Node *new_node = (Node *) malloc(sizeof(Node));
    new_node->key = key;
    new_node->color = color;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->parent = NULL;
    return new_node;
}

RedBlackTree *initializeRedBlackTree() {
    RedBlackTree *tree = (RedBlackTree *) malloc(sizeof(RedBlackTree));
    tree->root = NULL;
    return tree;
}

void leftRotate(RedBlackTree *tree, Node *x) {
    Node *y = x->right;
    x->right = y->left;
    if (y->left != NULL) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        tree->root = y;
    }
    else if (x == x->parent->left) {
        x->parent->left = y;
    }
    else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

void rightRotate(RedBlackTree *tree, Node *x) {
    Node *y = x->left;
    x->left = y->right;
    if (y->right != NULL) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        tree->root = y;
    }
    else if (x == x->parent->left) {
        x->parent->left = y;
    }
    else {
        x->parent->right = y;
    }
    y->right = x;
    x->parent = y;
}

void insertFixup(RedBlackTree *tree, Node *new_node) {
    while (new_node->parent != NULL && new_node->parent->color == RED) {
        if (new_node->parent == new_node->parent->parent->left) {
            Node *y = new_node->parent->parent->right;
            if (y != NULL && y->color == RED) {
                new_node->parent->color = BLACK;
                y->color = BLACK;
                new_node->parent->parent->color = RED;
                new_node = new_node->parent->parent;
            }
            else {
                if (new_node == new_node->parent->right) {
                    new_node = new_node->parent;
                    leftRotate(tree, new_node);
                }
                new_node->parent->color = BLACK;
                new_node->parent->parent->color = RED;
                rightRotate(tree, new_node->parent->parent);
            }
        }
        else {
            Node *y = new_node->parent->parent->left;
            if (y != NULL && y->color == RED) {
                new_node->parent->color = BLACK;
                y->color = BLACK;
                new_node->parent->parent->color = RED;
                new_node = new_node->parent->parent;
            }
            else {
                if (new_node == new_node->parent->left) {
                    new_node = new_node->parent;
                    rightRotate(tree, new_node);
                }
                new_node->parent->color = BLACK;
                new_node->parent->parent->color = RED;
                leftRotate(tree, new_node->parent->parent);
            }
        }
    }
    tree->root->color = BLACK;
}

bool insert(RedBlackTree *tree, size_t key) {
    if (search(tree->root, key)) return false;
    Node *new_node = createNode(key, RED);
    Node *y = NULL;
    Node *x = tree->root;
    while (x != NULL) {
        y = x;
        if (new_node->key < x->key) {
            x = x->left;
        }
        else {
            x = x->right;
        }
    }
    new_node->parent = y;
    if (y == NULL) {
        tree->root = new_node;
    }
    else if (new_node->key < y->key) {
        y->left = new_node;
    }
    else {
        y->right = new_node;
    }
    insertFixup(tree, new_node);
    return true;
}

bool search(Node *root, size_t key) {
    if (root == NULL) return false;
    if (root->key == key)
        return true;
    if (key < root->key) {
        return search(root->left, key);
    }
    else {
        return search(root->right, key);
    }
}

void freeNode(Node *node) {
    if (node == NULL) {
        return;
    }
    freeNode(node->left);
    freeNode(node->right);
    free(node);
}

void clearRedBlackTree(RedBlackTree *tree) {
    if (tree == NULL) return;
    freeNode(tree->root);
    tree->root = NULL;
}

void destroyRedBlackTree(RedBlackTree *tree) {
    freeNode(tree->root);
    free(tree);
}