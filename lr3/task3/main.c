#include "../status.h"
#include "tree.h"
#include <stdio.h>

// unsigned char - целочисленные значения в диапазоне [0; 255]
status_t parseHuffmanTree(char const *path, Node ** const out) {
    if (path == NULL)
        return STATUS_ERR_NULL_ARG;

    static const size_t UCHARCAP = 256;

    FILE *f = fopen(path, "rb");
    if (!f)
        return STATUS_ERR_OPEN;

    size_t *freq = calloc(UCHARCAP, sizeof *freq);
    if (!freq) {
        fclose(f);
        return STATUS_ERR_ALLOC;
    }

    int c;
    int nodecap = 0;
    while ((c = fgetc(f)) != EOF) {
        unsigned char u = (unsigned char)c;
        if (freq[u] == 0) nodecap += 1;
        freq[u] += 1;
    }

    
    if (fclose(f) != 0) {
        free(freq);
        return STATUS_ERR_CLOSE;
    }

    Node *nodes[nodecap];
    int n = 0;
    for (int i = 0; i < UCHARCAP; i++) {
        if (freq[i] != 0) {
            Node *node = newNode((unsigned char)i, freq[i]);
            if (!node) {
                for (int j = i; j >= 0; j--)
                    free(nodes[j]);
                free(freq);
                return STATUS_ERR_ALLOC;
            }
            nodes[n++] = node;
        }
    }
    free(freq);

    Node *huffmanNode = buildHuffmanTree(nodes, nodecap);
    if (!huffmanNode) return STATUS_ERR_ALLOC;
    *out = huffmanNode;

    return STATUS_OK;
}

void findAndPrint(Node* root, unsigned char target, int arr[], int top) {
    if (root == NULL) return;

    if (!root->left && !root->right) {
        if (root->ch == target) {
            printf("%3u (%c): %3zu раз | ", root->ch, root->ch, root->freq);
            if (top == 0) printf("0");
            for (int i = 0; i < top; i++) printf("%d", arr[i]);
            printf("\n");
        }
        return;
    }
    
    if (root->left) {
        arr[top] = 0;
        findAndPrint(root->left, target, arr, top + 1);
    }
    if (root->right) {
        arr[top] = 1;
        findAndPrint(root->right, target, arr, top + 1);
    }
}

void printCodes(Node* root, int arr[]) {
    for (int i = 0; i < 256; i++) {
        findAndPrint(root, (unsigned char)i, arr, 0);
    }
}


void get_stats(Node *root, int depth, size_t *total_bits, size_t *total_orig_bytes) {
    if (root == NULL) return;

    if (!root->left && !root->right) {
        *total_bits += root->freq * depth;
        *total_orig_bytes += root->freq;
        return;
    }

    get_stats(root->left, depth + 1, total_bits, total_orig_bytes);
    get_stats(root->right, depth + 1, total_bits, total_orig_bytes);
}


int main(int argc, char *argv[]) {
    Node *root;
    parseHuffmanTree(argv[1], &root);

    int arr[BUFSIZ], top = 0;
    printf("Таблица кодов Хаффмана:\n");
    printCodes(root, arr);

    size_t total_bits = 0;
    size_t total_orig_bytes = 0;
    
    get_stats(root, 0, &total_bits, &total_orig_bytes);

    size_t total_encoded_bytes = (total_bits + 7) / 8;

    double savings = 0.0;
    if (total_orig_bytes > 0) {
        savings = 100.0 * (1.0 - (double)total_encoded_bytes / (double)total_orig_bytes);
    }
    printf("Размер исходного файла:       %5zu байт (%zu бит)\n", 
        total_orig_bytes, total_orig_bytes * 8);
    printf("Размер закодированных данных: %5zu байт (%zu бит)\n", 
        total_encoded_bytes, total_bits);
    printf("Экономия: %.2f%%\n", savings);

    freeTree(root);

    return 0;
}