#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define BUFFER_SIZE 5000
#define FIFO1 "/tmp/fifo1"
#define FIFO2 "/tmp/fifo2"

void replace_vowels(char *input, char *output) {
    int i, j = 0;
    char hex[10];
    for (i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case 'a': case 'e': case 'i': case 'o': case 'u':
            case 'A': case 'E': case 'I': case 'O': case 'U':
                sprintf(hex, "{0x%X}", input[i]);
                strcpy(&output[j], hex);
                j += strlen(hex);
                break;
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Создание именованных каналов
    if (mkfifo(FIFO1, 0666) == -1) {
        perror("mkfifo FIFO1");
        exit(EXIT_FAILURE);
    }
    if (mkfifo(FIFO2, 0666) == -1) {
        perror("mkfifo FIFO2");
        exit(EXIT_FAILURE);
    }

    pid_t pid;

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Процесс 2: обработка данных
        int fifo1_fd = open(FIFO1, O_RDONLY);
        char buffer[BUFFER_SIZE];
        char output_buffer[BUFFER_SIZE * 5];
        ssize_t bytes_read = read(fifo1_fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("read from FIFO1");
            exit(EXIT_FAILURE);
        }
        close(fifo1_fd);

        replace_vowels(buffer, output_buffer);

        int fifo2_fd = open(FIFO2, O_WRONLY);
        write(fifo2_fd, output_buffer, strlen(output_buffer) + 1);
        close(fifo2_fd);
        exit(EXIT_SUCCESS);
    } else {
        // Процесс 1: чтение из файла и запись в файл
        int input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1) {
            perror("open input file");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(input_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read == -1) {
            perror("read");
            close(input_fd);
            exit(EXIT_FAILURE);
        }
        buffer[bytes_read] = '\0';
        close(input_fd);

        int fifo1_fd = open(FIFO1, O_WRONLY);
        write(fifo1_fd, buffer, bytes_read + 1);
        close(fifo1_fd);

        int fifo2_fd = open(FIFO2, O_RDONLY);
        int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            perror("open output file");
            exit(EXIT_FAILURE);
        }

        bytes_read = read(fifo2_fd, buffer, sizeof(buffer));
        if (bytes_read == -1) {
            perror("read from FIFO2");
            close(output_fd);
            exit(EXIT_FAILURE);
        }
        close(fifo2_fd);

        write(output_fd, buffer, bytes_read);
        close(output_fd);

        // Удаление именованных каналов
        unlink(FIFO1);
        unlink(FIFO2);

        exit(EXIT_SUCCESS);
    }

    return 0;
}
