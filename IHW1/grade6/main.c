#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 5000

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

    int pipe1[2], pipe2[2];
    pid_t pid;

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Процесс 2: обработка данных
        close(pipe1[1]);
        close(pipe2[0]);

        char buffer[BUFFER_SIZE];
        char output_buffer[BUFFER_SIZE * 5];
        ssize_t bytes_read = read(pipe1[0], buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("read from pipe1");
            exit(EXIT_FAILURE);
        }
        close(pipe1[0]);

        replace_vowels(buffer, output_buffer);

        write(pipe2[1], output_buffer, strlen(output_buffer) + 1);
        close(pipe2[1]);
        exit(EXIT_SUCCESS);
    } else {
        // Процесс 1: чтение из файла и запись в файл
        close(pipe1[0]);
        close(pipe2[1]);

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

        write(pipe1[1], buffer, bytes_read + 1);
        close(pipe1[1]);

        int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            perror("open output file");
            exit(EXIT_FAILURE);
        }

        bytes_read = read(pipe2[0], buffer, sizeof(buffer));
        if (bytes_read == -1) {
            perror("read from pipe2");
            close(output_fd);
            exit(EXIT_FAILURE);
        }
        close(pipe2[0]);

        write(output_fd, buffer, bytes_read);
        close(output_fd);
        exit(EXIT_SUCCESS);
    }

    return 0;
}
