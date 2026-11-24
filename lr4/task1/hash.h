#ifndef HASH_H
#define HASH_H

#include "../status.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct HashNode  HashNode;
typedef struct HashTable HashTable;

struct HashNode {
    char *key;
    char *value;
    struct HashNode *next;
};

struct HashTable {
    size_t size;
    HashNode **buckets;
};

/* HashNode */
status_t HashNode_create(char *key, char *value, HashNode **out);
status_t HashNode_clone(HashNode *node, HashNode **out);
void     HashNode_free(HashNode *node, bool recursive);

/* HashTable*/
status_t HashTable_create(size_t size, HashTable **out);
status_t HashTable_add_HashNode(HashTable *table, HashNode *node);
status_t HashTable_find_HashNode(HashTable *table, char *key, HashNode **out);
status_t HashTable_resize(HashTable *table, HashTable **out);
void     HashTable_free(HashTable *table);

#endif