#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compress.h"
#include "huffman.h"
#include "utils.h"

#define BUFFER_SIZE 8192

// Функция для подсчёта частот появления каждого символа в файле
// freq — массив из 256 элементов, каждый элемент хранит количество вхождений символа с соответствующим ASCII-кодом
// total_bytes — общее число считанных байт из файла
static int count_frequencies(const char *filename, unsigned long freq[256], unsigned long *total_bytes) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", filename);
        return 0;
    }
    
    memset(freq, 0, sizeof(unsigned long) * 256);
    *total_bytes = 0;
    
    unsigned char buffer[BUFFER_SIZE]; // буфер для чтения порциями
    size_t read_bytes;

    while ((read_bytes = fread(buffer, 1, BUFFER_SIZE, f)) > 0) {
        // Для каждого прочитанного байта увеличиваем соответствующую частоту
        for (size_t i = 0; i < read_bytes; i++)
            freq[buffer[i]]++;
        // Увеличиваем общее количество прочитанных байт
        *total_bytes += read_bytes;

        // Печатаем прогресс
        print_progress(*total_bytes, *total_bytes);
    }
    fclose(f);
    return 1;
}

// Основная функция сжатия файла с помощью алгоритма Хаффмана
int compress_file(const char *in, const char *out) {
    unsigned long freq[256];
    unsigned long total_bytes = 0;   // общее количество байт во входном файле

    printf("Counting frequencies...\n");
    if (!count_frequencies(in, freq, &total_bytes)) // считаем частоты символов
        return 1;
    if (total_bytes == 0) {
        fprintf(stderr, "Input file is empty\n");
        return 1;
    }

    printf("Building Huffman tree...\n");
    // Строим дерево Хаффмана на основе подсчитанных частот
    HuffmanNode *root = build_huffman_tree(freq);
    if (!root) {
        fprintf(stderr, "Error building Huffman tree\n");
        return 1;
    }

    // Массив для хранения кодов Хаффмана для каждого символа
    HuffmanCode codes[256];
    generate_codes(root, codes); // генерируем коды по дереву

    FILE *fin = fopen(in, "rb");
    if (!fin) {
        fprintf(stderr, "Error opening input file '%s'\n", in);
        free_huffman_tree(root);
        return 1;
    }

    FILE *fout = fopen(out, "wb");
    if (!fout) {
        fprintf(stderr, "Error opening output file '%s'\n", out);
        fclose(fin);
        free_huffman_tree(root);
        return 1;
    }

    // Сначала записываем сериализованное дерево Хаффмана в выходной файл,
    // чтобы при распаковке знать, как декодировать байты
    write_tree(fout, root);

    // Записываем размер исходного файла (для распаковки)
    fwrite(&total_bytes, sizeof(unsigned long), 1, fout);

    unsigned char inbuf[BUFFER_SIZE]; // буфер для чтения входного файла
    unsigned char outbuf = 0;          // буфер для битового вывода
    int outbits = 0;                   // количество бит, записанных в outbuf!

    unsigned long read_bytes_total = 0; // количество уже обработанных байт из входного файла
    size_t read_bytes;

    printf("Encoding file:\n");

    // Читаем входной файл блоками по BUFFER_SIZE байт.
    // Для каждого считанного символа берём соответствующий ему код Хаффман а(последовательность битов).
    // Побитово записываем эти коды в выходной буфер(outbuf), сдвигая биты влево и добавляя новый бит.
    // Как только в выходном байте накопится 8 бит, записываем этот байт в выходной файл и обнуляем буфер.
    // Повторяем процесс для всех символов блока, затем читаем следующий блок.
    while ((read_bytes = fread(inbuf, 1, BUFFER_SIZE, fin)) > 0) { 
        for (size_t i = 0; i < read_bytes; i++) { 
            HuffmanCode *code = &codes[inbuf[i]]; 
            for (int b = 0; b < code->length; b++) {
                outbuf <<= 1;
                outbuf |= code->bits[b];
                outbits++;

                if (outbits == 8) {
                    fwrite(&outbuf, 1, 1, fout);
                    outbuf = 0;
                    outbits = 0;
                }
            }
        }
        read_bytes_total += read_bytes;
        print_progress(read_bytes_total, total_bytes);
    }

    // Если остались ещё биты, не доведённые до полного байта,
    // дописываем справа нулями, чтобы записать последний байт
    if (outbits > 0) {
        outbuf <<= (8 - outbits);
        fwrite(&outbuf, 1, 1, fout);
    }

    fclose(fin);
    fclose(fout);
    free_huffman_tree(root);

    unsigned long outsize = file_size(out); // узнаём размер сжатого файла

    // Выводим статистику сжатия
    printf("\nCompression result:\n");
    printf("Input size:        %lu bytes\n", total_bytes);
    printf("Output size:       %lu bytes\n", outsize);
    printf("Compression ratio: %.2f\n", (double)total_bytes / outsize);

    return 0;
}
