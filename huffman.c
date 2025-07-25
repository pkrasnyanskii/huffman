#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"

// Структура минимальной кучи (минимальная по частоте встречаемости символов)
typedef struct {
    HuffmanNode **nodes; // массив указателей на узлы дерева Хаффмана
    int size;
    int capacity;
} MinHeap;

static MinHeap* create_min_heap(int capacity) {
    MinHeap *heap = malloc(sizeof(MinHeap)); // выделяем память под структуру кучи
    heap->nodes = malloc(sizeof(HuffmanNode*) * capacity);
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

static void swap_nodes(HuffmanNode **a, HuffmanNode **b) {
    HuffmanNode *t = *a;
    *a = *b;
    *b = t;
}

// Восстановление свойства минимальной кучи для поддерева с корнем в idx
static void min_heapify(MinHeap *heap, int idx) {
    int smallest = idx;                     // предполагаем, что корень - наименьший
    int left = 2*idx + 1;
    int right = 2*idx + 2;
    // Если левый потомок существует и его частота меньше частоты текущего наименьшего
    if (left < heap->size && heap->nodes[left]->freq < heap->nodes[smallest]->freq)
        smallest = left;
    if (right < heap->size && heap->nodes[right]->freq < heap->nodes[smallest]->freq)
        smallest = right;
    // Если наименьший элемент не корень - меняем местами и рекурсивно вызываем для изменённого поддерева
    if (smallest != idx) {
        swap_nodes(&heap->nodes[smallest], &heap->nodes[idx]);
        min_heapify(heap, smallest);
    }
}

// Извлечение минимального элемента (корня кучи)
static HuffmanNode* extract_min(MinHeap *heap) {
    HuffmanNode *temp = heap->nodes[0];
    heap->nodes[0] = heap->nodes[--heap->size]; // заменяем корень последним элементом
    min_heapify(heap, 0);
    return temp;
}


// Пока не достигли корня (i != 0) и частота текущего узла меньше частоты родительского,
// сдвигаем родительский узел вниз, освобождая место для вставки нового узла.
// После сдвига переходим к позиции родителя, повторяя проверку вверх по дереву.
// В конце вставляем новый узел на подходящее место, поддерживая свойства мин-кучи.
static void insert_min_heap(MinHeap *heap, HuffmanNode *node) {
    int i = heap->size++;
    while (i && node->freq < heap->nodes[(i-1)/2]->freq) {
        heap->nodes[i] = heap->nodes[(i-1)/2];
        i = (i-1)/2;
    }
    heap->nodes[i] = node;
}

// Дерево Хаффмана на массиве частот
HuffmanNode* build_huffman_tree(unsigned long freq[256]) {
    MinHeap *heap = create_min_heap(256); // 1 byte

    // Для каждого возможного символа
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            HuffmanNode *node = malloc(sizeof(HuffmanNode));
            node->symbol = (unsigned char)i;
            node->freq = freq[i];
            node->left = node->right = NULL;
            insert_min_heap(heap, node);
        }
    }

    if (heap->size == 0) {
        free(heap->nodes);
        free(heap);
        return NULL;
    }

    // Пока в куче больше одного узла — строим дерево
    while (heap->size > 1) {
        HuffmanNode *left = extract_min(heap);  // самый редкий символ
        HuffmanNode *right = extract_min(heap); // следующий по редкости

        HuffmanNode *parent = malloc(sizeof(HuffmanNode));
        parent->symbol = 0;
        parent->freq = left->freq + right->freq; // сумма частот потомков
        parent->left = left;                     // левый потомок
        parent->right = right;                   // правый потомок

        insert_min_heap(heap, parent); // вставляем новый узел обратно в кучу
    }

    HuffmanNode *root = extract_min(heap); // корень дерева Хаффмана
    free(heap->nodes);
    free(heap);
    return root;
}

void free_huffman_tree(HuffmanNode *root) {
    if (!root) return;
    free_huffman_tree(root->left);
    free_huffman_tree(root->right);
    free(root);
}

// Рекурсивная функция для генерации кодов Хаффмана для всех символов
static void generate_codes_rec(HuffmanNode *node, HuffmanCode codes[256], unsigned char *path, int depth) {
    if (!node->left && !node->right) { // если это лист (символ)
        codes[node->symbol].length = depth; // длина кода
        for (int i = 0; i < depth; i++) // копируем путь (последовательность битов) в код
            codes[node->symbol].bits[i] = path[i];
        return;
    }
    if (node->left) { // идём влево
        path[depth] = 0;
        generate_codes_rec(node->left, codes, path, depth + 1);
    }
    if (node->right) { // идём вправо
        path[depth] = 1;
        generate_codes_rec(node->right, codes, path, depth + 1);
    }
}

// Основная функция генерации кодов Хаффмана
void generate_codes(HuffmanNode *root, HuffmanCode codes[256]) {
    for (int i = 0; i < 256; i++)
        codes[i].length = 0;
    unsigned char path[256]; // путь от корня к листу (битовый код)
    generate_codes_rec(root, codes, path, 0);
}

// Запись дерева Хаффмана в файл (префиксный обход)
// записываем 1, если это лист (и следом сам символ), иначе записываем 0
void write_tree(FILE *f, HuffmanNode *root) {
    if (!root) return;
    if (!root->left && !root->right) {
        fputc('1', f);
        fputc(root->symbol, f);
    } else {
        fputc('0', f);
        write_tree(f, root->left);     // рекурсивно левое поддерево
        write_tree(f, root->right);    // рекурсивно правое поддерево
    }
}

// Чтение дерева Хаффмана из файла:
// '1' + символ — лист; '0' — внутренний узел, за которым следуют два поддерева
HuffmanNode* read_tree(FILE *f) {
    int flag = fgetc(f); // считываем маркер
    if (flag == EOF) return NULL;

    HuffmanNode *node = malloc(sizeof(HuffmanNode));

    if (flag == '1') { // если лист
        int sym = fgetc(f); // читаем символ
        if (sym == EOF) {
            free(node);
            return NULL;
        }
        node->symbol = (unsigned char)sym;
        node->freq = 0;
        node->left = node->right = NULL; // это лист
    } else if (flag == '0') {     // если внутренний узел
        node->left = read_tree(f);  // читаем левое поддерево
        node->right = read_tree(f); // читаем правое поддерево
    } else {
        free(node);
        return NULL;
    }
    return node;
}
