#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <stdio.h>

typedef struct HuffmanNode {
    unsigned char symbol;
    unsigned long freq;
    struct HuffmanNode *left, *right;
} HuffmanNode;

typedef struct {
    int length;
    unsigned char bits[256];
} HuffmanCode;

HuffmanNode* build_huffman_tree(unsigned long freq[256]);
void free_huffman_tree(HuffmanNode *root);
void generate_codes(HuffmanNode *root, HuffmanCode codes[256]);
void write_tree(FILE *f, HuffmanNode *root);
HuffmanNode* read_tree(FILE *f);

#endif