#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // Проверка правильности аргументов командной строки
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }
    
    // Открытие входного файла для чтения
    int input_fd = open(argv[1], O_RDONLY);
    if (input_fd == -1) {
        perror("Error opening input file");
        return 1;
    }
    
    // Открытие выходного файла для записи, создание нового файла при необходимости
    int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (output_fd == -1) {
        perror("Error opening output file");
        close(input_fd);
        return 1;
    }

    // Буфер для чтения и записи данных
    char buffer[BUFFER_SIZE];
    
    // Цикл для чтения из входного файла и записи в выходной файл
    ssize_t bytes_read;
    ssize_t bytes_written;
    while ((bytes_read = read(input_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(output_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Error writing to output file");
            close(input_fd);
            close(output_fd);
            return 1;
        }
    }
    
    // Проверка и копирование прав доступа для исполняемых файлов
    struct stat st;
    fstat(input_fd, &st);
    if (st.st_mode & S_IXUSR) {
        fchmod(output_fd, st.st_mode);
    }
    
    // Закрытие файлов
    close(input_fd);
    close(output_fd);

    return 0;
}
