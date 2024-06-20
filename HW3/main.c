#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>
#include <inttypes.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>

// Функция для вычисления n-го числа Фибоначчи рекурсивно.
unsigned long long int fibonacci(unsigned long long int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n-1) + fibonacci(n-2);
}

// Функция для вычисления факториала числа n рекурсивно.
unsigned long long int factorial(unsigned long long int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n-1);
}

// Процесс потомка
void child_process(unsigned long long int n) {
    printf("Child Process: PID=%d, Parent PID=%d\n", getpid(), getppid());
    // Максимальное значение факториала, которое можно сохранить в unsigned long long int, это 20!.
    if (n > 20) {
        printf("Overflow while calculating factorial!\n");
        return;
    }
    printf("%llu! = %llu\n", n, factorial(n));
}

// Процесс отображения содержимого каталога
void catalog_process() {
    printf("\nCurrent directory contents:\n");
    DIR *dir;
    struct dirent *entry;
    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        return 1;
    }

    unsigned long long int n = strtoull(argv[1], NULL, 10);
    printf("Parent Process: PID=%d, Parent PID=%d\n", getpid(), getppid());
    pid_t pid = fork();

    // Обработка ошибки при вызове fork.
    if (pid == -1) {
        perror("Fork");
        return 1;
    }
    // Ветка процесса потомка -> вычисление факториала
    else if (pid == 0) {
        child_process(n);
        exit(0);
    }
    // Ветка процесса родителя -> вычисление числа Фибоначчи и отображение каталога
    else {
        wait(NULL);
        printf("Fib(%llu) = %llu\n", n, fibonacci(n));
        catalog_process();
    }
    return 0;
}
