#include <stdio.h>
#include <stdlib.h>
#include "decompress.h"
#include "huffman.h"
#include "utils.h"

#define INPUT_BUF_SIZE 65536     // Буфер для чтения закодированных данных из файла
#define OUTPUT_BUF_SIZE 65536    // Буфер для записи декодированных данных в файл

int decompress_file(const char *in, const char *out) {
    FILE *fin = fopen(in, "rb");
    if (!fin) {
        fprintf(stderr, "Error: cannot open input file '%s'\n", in);
        return 1;
    }

    FILE *fout = fopen(out, "wb");
    if (!fout) {
        fprintf(stderr, "Error: cannot open output file '%s'\n", out);
        fclose(fin);
        return 1;
    }

    // Считываем дерево Хаффмана из входного файла
    HuffmanNode *root = read_tree(fin);
    if (!root) {
        fprintf(stderr, "Error reading Huffman tree from archive\n");
        fclose(fin);
        fclose(fout);
        return 1;
    }

    // Считываем размер исходного файла
    unsigned long original_size = 0;
    if (fread(&original_size, sizeof(unsigned long), 1, fin) != 1) {
        fprintf(stderr, "Error reading original file size\n");
        free_huffman_tree(root);
        fclose(fin);
        fclose(fout);
        return 1;
    }

    // Буфер для чтения сжатых данных
    unsigned char input_buf[INPUT_BUF_SIZE];
    size_t input_pos = 0, input_size = 0; // текущая позиция и количество данных в буфере
    int bit_pos = 8;                      // позиция текущего бита (от 8 до 1)

    // Буфер для накопления распакованных данных
    unsigned char output_buf[OUTPUT_BUF_SIZE];
    size_t output_pos = 0;                // позиция записи в выходной буфер

    unsigned long decoded = 0;
    HuffmanNode *current = root;           // текущий узел

    printf("Decoding file:\n");

    // Пока не распаковали весь исходный файл
    while (decoded < original_size) {
        // Если входной буфер пуст, читаем очередную порцию данных из файла
        if (input_pos >= input_size) {
            input_size = fread(input_buf, 1, INPUT_BUF_SIZE, fin);
            if (input_size == 0) {
                fprintf(stderr, "Unexpected end of file\n");
                free_huffman_tree(root);
                fclose(fin);
                fclose(fout);
                return 1;
            }
            input_pos = 0;   // сбрасываем позицию в буфере
            bit_pos = 8;     // начинаем с младшего бита нового байта
        }

        // Извлекаем текущий бит из входного буфера (сдвигаем вправо и маскируем)
        int bit = (input_buf[input_pos] >> (bit_pos - 1)) & 1;
        bit_pos--;
        if (bit_pos == 0) {
            input_pos++;    // переходим к следующему байту
            bit_pos = 8;
        }

        // Переходим в дереве Хаффмана: вправо если бит=1, влево если 0
        current = bit ? current->right : current->left;

        // Если достигли листа (символа)
        if (!current->left && !current->right) {
            output_buf[output_pos++] = current->symbol; // записываем символ в выходной буфер
            decoded++;
            current = root;                             // возвращаемся к корню дерева

            // Если буфер заполнен, записываем его в файл и сбрасываем позицию
            if (output_pos == OUTPUT_BUF_SIZE) {
                fwrite(output_buf, 1, OUTPUT_BUF_SIZE, fout);
                output_pos = 0;
            }

            // Периодически выводим прогресс (каждые 1024 байта и при окончании)
            if (decoded % 1024 == 0 || decoded == original_size)
                print_progress(decoded, original_size);
        }
    }

    // Записываем оставшиеся данные из буфера, если они есть
    if (output_pos > 0)
        fwrite(output_buf, 1, output_pos, fout);

    free_huffman_tree(root);
    fclose(fin);
    fclose(fout);

    printf("\nDecompression completed\n");

    return 0;
}
