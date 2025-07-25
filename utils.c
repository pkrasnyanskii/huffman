#include <stdio.h>
#include <sys/stat.h>
#include "utils.h"

// Печатает прогресс выполнения в процентах на одной строке
void print_progress(unsigned long current, unsigned long total) {
    if (total == 0) return;  // Защита от деления на ноль
    int percent = (int)(current * 100 / total);
    printf("\rProgress: %3d%%\n", percent);
    fflush(stdout);  // Сбрасываем буфер stdout для немедленного вывода
}

// Возвращает размер файла в байтах
unsigned long file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) // Если удалось получить информацию о файле
        return st.st_size;        // Возвращаем размер файла
    return 0;
}
