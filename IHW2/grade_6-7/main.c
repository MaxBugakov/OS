#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>

#define ARRAY_SIZE 10

int shm_id;
int sem_id;
int *shared_memory;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

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

void reader(int reader_id) {
    while (1) {
        sleep(rand() % 3 + 1);  // simulate some delay
        sem_op(sem_id, 0, -1);

        int index = rand() % ARRAY_SIZE;
        int value = shared_memory[index];
        
        printf("Reader %d: Index: %d, Value: %d, ", reader_id, index, value);
        fibonacci(value);

        sem_op(sem_id, 0, 1);
    }
}

void writer(int writer_id) {
    while (1) {
        sleep(rand() % 3 + 1);  // simulate some delay
        sem_op(sem_id, 1, -1);

        int index = rand() % ARRAY_SIZE;
        int old_value = shared_memory[index];
        int new_value = rand() % 100;

        shared_memory[index] = new_value;

        printf("Writer %d: Index: %d, Old Value: %d, New Value: %d\n", writer_id, index, old_value, new_value);

        sem_op(sem_id, 1, 1);
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

    shm_id = shmget(IPC_PRIVATE, ARRAY_SIZE * sizeof(int), IPC_CREAT | 0666);
    shared_memory = (int *)shmat(shm_id, NULL, 0);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        shared_memory[i] = i + 1;
    }

    sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
    union semun sem_union;
    sem_union.val = 1;
    semctl(sem_id, 0, SETVAL, sem_union);
    semctl(sem_id, 1, SETVAL, sem_union);

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

    semctl(sem_id, 0, IPC_RMID, sem_union);
    shmdt(shared_memory);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
