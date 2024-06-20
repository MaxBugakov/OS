#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>

#define SHARED_MEMORY_NAME "/shared_memory"
#define SEM_READER_NAME "/sem_reader"
#define SEM_WRITER_NAME "/sem_writer"
#define ARRAY_SIZE 10

int *shared_memory;
sem_t *sem_reader;
sem_t *sem_writer;

void fibonacci(int n) {
    if (n == 0) {
        printf("Fibonacci: 0\n");
        return;
    }
    int a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        int temp = a + b;
        a = b;
        b = temp;
    }
    printf("Fibonacci: %d\n", b);
}

void reader(int reader_id) {
    while (1) {
        sleep(rand() % 3 + 1);  // simulate some delay
        sem_wait(sem_reader);
        
        int index = rand() % ARRAY_SIZE;
        int value = shared_memory[index];
        
        printf("Reader %d: Index: %d, Value: %d, ", reader_id, index, value);
        fibonacci(value);

        sem_post(sem_reader);
    }
}

void writer(int writer_id) {
    while (1) {
        sleep(rand() % 3 + 1);  // simulate some delay
        sem_wait(sem_writer);
        
        int index = rand() % ARRAY_SIZE;
        int old_value = shared_memory[index];
        int new_value = rand() % 100;

        shared_memory[index] = new_value;

        printf("Writer %d: Index: %d, Old Value: %d, New Value: %d\n", writer_id, index, old_value, new_value);

        sem_post(sem_writer);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_readers> <number_of_writers>\n", argv[0]);
        exit(1);
    }

    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);

    srand(time(NULL));

    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, ARRAY_SIZE * sizeof(int));
    shared_memory = mmap(0, ARRAY_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    for (int i = 0; i < ARRAY_SIZE; i++) {
        shared_memory[i] = i + 1;
    }

    sem_reader = sem_open(SEM_READER_NAME, O_CREAT, 0666, 1);
    sem_writer = sem_open(SEM_WRITER_NAME, O_CREAT, 0666, 1);

    pid_t pids[num_readers + num_writers];

    for (int i = 0; i < num_readers; i++) {
        if ((pids[i] = fork()) == 0) {
            reader(i + 1);
            exit(0);
        }
    }

    for (int i = 0; i < num_writers; i++) {
        if ((pids[num_readers + i] = fork()) == 0) {
            writer(i + 1);
            exit(0);
        }
    }

    for (int i = 0; i < num_readers + num_writers; i++) {
        wait(NULL);
    }

    sem_close(sem_reader);
    sem_close(sem_writer);
    sem_unlink(SEM_READER_NAME);
    sem_unlink(SEM_WRITER_NAME);

    munmap(shared_memory, ARRAY_SIZE * sizeof(int));
    close(shm_fd);
    shm_unlink(SHARED_MEMORY_NAME);

    return 0;
}
