#include "hash.h"
#include "../status.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static size_t hash_func(char *key, size_t size) {
    size_t hash = 0;
    size_t digit = 0; 
    while(*key != '\0') {
        if      ('0' <= *key && *key <= '9') digit = *key - '0';
        else if ('A' <= *key && *key <= 'Z') digit = *key - 'A' + 10;
        else if ('a' <= *key && *key <= 'z') digit = *key - 'a' + 36;
        else    return SIZE_MAX;
        hash = hash * 62 + digit;
        key++;
    }

    return hash % size;
}

status_t HashNode_create(char *key, char *value, HashNode **out) {
    if (!key || !value || !out) return STATUS_ERR_NULL_ARG;

    HashNode *node = (HashNode *)malloc(sizeof(*node));
    if (!node) return STATUS_ERR_ALLOC;

    node->key = strdup(key);
    node->value = strdup(value);
    if (!node->key || !node->value) {
        free(node->key);
        free(node->value);
        free(node);
        return STATUS_ERR_ALLOC;
    }
    node->next = NULL;

    *out = node;

    return STATUS_OK;    
}

status_t HashNode_clone(HashNode *node, HashNode **out) {
    if (!node || !out) return STATUS_ERR_NULL_ARG;

    status_t rc = HashNode_create(node->key, node->value, out);
    
    return rc;
}

void HashNode_free(HashNode *node, bool recursive) {
    if (!node) return;
    if (recursive) HashNode_free(node->next, true);
    free(node->key);
    free(node->value);
    free(node);
}

status_t HashTable_create(size_t size, HashTable **out) {
    if (!out) return STATUS_ERR_NULL_ARG;
    if (size == 0) return STATUS_ERR_INVALID; 
    
    HashTable *table = (HashTable *)malloc(sizeof(HashTable));
    if (!table) return STATUS_ERR_ALLOC; 
    
    table->size = size;
    table->buckets = (HashNode **)calloc(size, sizeof(HashNode*));
    if (!table->buckets) {
        free(table);
        return STATUS_ERR_ALLOC;
    }
    
    *out = table;

    return STATUS_OK;
}

static size_t get_chain_len(HashNode *node) {
    size_t len = 0;
    while (node) {
        node = node->next;
        len++;
    }
    return len;
}

static bool HashTable_need_resize(HashTable *table) {
    size_t min = SIZE_MAX;
    size_t max = 0;

    for (size_t i = 0; i < table->size; i++) {
        size_t len = get_chain_len(table->buckets[i]);

        if (len == 0) continue;

        if (len < min) min = len;
        if (len > max) max = len;
    }

    return max / min >= 2;
}

static status_t HashTable_insert_raw(HashTable *table, HashNode *node) {
    if (!table || !node) return STATUS_ERR_NULL_ARG;
    if (node->next != NULL) return STATUS_ERR_INVALID;

    size_t hash = hash_func(node->key, table->size);
    if (hash == SIZE_MAX) return STATUS_ERR_INVALID;

    node->next = table->buckets[hash];
    table->buckets[hash] = node;

    return STATUS_OK;
}

status_t HashTable_add_HashNode(HashTable *table, HashNode *node) {
    if (!table || !node) return STATUS_ERR_NULL_ARG;

    status_t rc = HashTable_insert_raw(table, node);
    if (rc) return rc;

    return HashTable_need_resize(table)
        ? STATUS_NEED_RESIZE
        : STATUS_OK;
}

status_t HashTable_find_HashNode(HashTable *table, char *key, HashNode **out) {
    if (!table || !key || !out) return STATUS_ERR_NULL_ARG;

    size_t hash = hash_func(key, table->size);
    if (hash == SIZE_MAX) return STATUS_ERR_INVALID;

    HashNode *node = table->buckets[hash];
    while (node != NULL) {
        if (strcmp(key, node->key) == 0) {
            *out = node;
            return STATUS_OK;
        }
        node = node->next;
    }

    return STATUS_ERR_NOT_FOUND;
}

status_t HashTable_resize(HashTable *table, HashTable **out) {
    if (!table || !out) return STATUS_ERR_NULL_ARG;

    size_t new_size = table->size;

    while (true) {
        if (new_size > SIZE_MAX / 2)
            return STATUS_ERR_OVERFLOW;

        new_size *= 2;

        HashTable *new_table = NULL;
        status_t rc = HashTable_create(new_size, &new_table);
        if (rc) return rc;

        for (size_t i = 0; i < table->size; i++) {
            for (HashNode *node = table->buckets[i];
                 node != NULL;
                 node = node->next) {

                HashNode *new_node = NULL;

                rc = HashNode_clone(node, &new_node);
                if (rc) {
                    HashTable_free(new_table);
                    return rc;
                }

                rc = HashTable_insert_raw(new_table, new_node);
                if (rc) {
                    HashNode_free(new_node, false);
                    HashTable_free(new_table);
                    return rc;
                }
            }
        }

        if (!HashTable_need_resize(new_table)) {
            *out = new_table;
            return STATUS_OK;
        }

        HashTable_free(new_table);
    }
}

void HashTable_free(HashTable *table) {
    if (!table) return;
    for (int i = 0; i < table->size; i++)
        HashNode_free(table->buckets[i], true);
    free(table->buckets);
    table->size = 0;
    free(table);
}