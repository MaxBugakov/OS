#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define ARRAY_SIZE 10
#define SHM_KEY 1234
#define SEM_KEY 5678

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int main() {
    int shm_id = shmget(SHM_KEY, ARRAY_SIZE * sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    int *shared_memory = (int *)shmat(shm_id, NULL, 0);
    if (shared_memory == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    for (int i = 0; i < ARRAY_SIZE; i++) {
        shared_memory[i] = i + 1;
    }

    int sem_id = semget(SEM_KEY, 2, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(1);
    }

    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        perror("semctl failed");
        exit(1);
    }
    if (semctl(sem_id, 1, SETVAL, sem_union) == -1) {
        perror("semctl failed");
        exit(1);
    }

    printf("Initialization complete. Press Enter to clean up and exit.\n");
    getchar();

    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) {
        perror("semctl remove failed");
    }
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl remove failed");
    }

    return 0;
}
