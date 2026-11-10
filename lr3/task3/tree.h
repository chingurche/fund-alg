#ifndef TREE_H
#define TREE_H

#include <stdlib.h>

typedef struct Node {
    unsigned char ch;
    size_t freq;
    struct Node *left, *right;
} Node;

typedef struct MinHeap {
    unsigned size;
    unsigned capacity;
    Node **array;
} MinHeap;

Node*          newNode(unsigned char ch, size_t freq);
MinHeap* createMinHeap(unsigned capacity);
void   swapMinHeapNode(Node** a, Node** b);
void        minHeapify(MinHeap* minHeap, int idx);
Node*       extractMin(MinHeap* minHeap);
void     insertMinHeap(MinHeap* minHeap, Node* minHeapNode);
void      buildMinHeap(MinHeap* minHeap);
Node* buildHuffmanTree(Node **nodes, int size);
void          freeTree(Node* root);

#endif