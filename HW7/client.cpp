#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>

pid_t pid;

int num;
int shmid;
char shar_object[] = "mem";

void sys_err(char *msg) {
    puts(msg);
    exit(1);
}

void ExitProgram(int sig) {
    if (shm_unlink(shar_object) == -1) {
        exit(0);
    }

    kill(pid, SIGINT);
    exit(0);
}

int main() {
    (void) signal(SIGINT, ExitProgram);
    pid_t my_pid = getpid();
    std::cout << "My pid: " << my_pid << "\n";
    printf("Enter server pid: ");
    std::cin >> pid;
    srand(time(NULL));
    if ((shmid = shm_open(shar_object, O_CREAT | O_RDWR, 0666)) == -1) {
        return 0;
    }
    if (ftruncate(shmid, sizeof(int)) == -1) {
        return 0;
    }
    int *msg_p = (int *) mmap(0, sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0);
    if (msg_p == (int *) -1) {
        return 0;
    }

    while (true) {
        num = random() % 100;
        *msg_p = num;
        printf("- %d\n", num);
        sleep(1);
    }
    close(shmid);
    return 0;
}