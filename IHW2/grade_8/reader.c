#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define ARRAY_SIZE 10
#define SHM_KEY 1234
#define SEM_KEY 5678

void sem_op(int sem_id, int sem_num, int op) {
    struct sembuf sem_op;
    sem_op.sem_num = sem_num;
    sem_op.sem_op = op;
    sem_op.sem_flg = 0;
    semop(sem_id, &sem_op, 1);
}

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <reader_id>\n", argv[0]);
        exit(1);
    }

    int reader_id = atoi(argv[1]);

    srand(time(NULL));

    int shm_id = shmget(SHM_KEY, ARRAY_SIZE * sizeof(int), 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    int *shared_memory = (int *)shmat(shm_id, NULL, 0);
    if (shared_memory == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    int sem_id = semget(SEM_KEY, 2, 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(1);
    }

    while (1) {
        sleep(rand() % 3 + 1);  // simulate some delay
        sem_op(sem_id, 0, -1);

        int index = rand() % ARRAY_SIZE;
        int value = shared_memory[index];
        
        printf("Reader %d: Index: %d, Value: %d, ", reader_id, index, value);
        fibonacci(value);

        sem_op(sem_id, 0, 1);
    }

    return 0;
}
