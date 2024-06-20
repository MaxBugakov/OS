#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define SHM_KEY 0x123 // ключ

struct SharedData {
    int data[5];
    bool ready;
    bool finished;
};

void cleanUp(int shmid, SharedData* sharedData) {
    // Отсоединение разделяемой памяти.
    if (shmdt(sharedData) == -1) {
        perror("shmdt");
    }
}

int main() {
    // Доступ к разделяемой памяти.
    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0644);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    // Присоединение.
    SharedData* sharedData = (SharedData*) shmat(shmid, NULL, 0);
    if (sharedData == (void*) -1) {
        perror("shmat");
        return 1;
    }

    srand(time(NULL));
    
    while (!sharedData->finished) {
        while (sharedData->ready) {
            sleep(1);
        }
        for (int i = 0; i < 5; i++) {
            sharedData->data[i] = rand() % 100;
            std::cout << "Sent: " << sharedData->data[i] << std::endl;
        }
        sharedData->ready = true;
    }

    cleanUp(shmid, sharedData);
    return 0;
}
