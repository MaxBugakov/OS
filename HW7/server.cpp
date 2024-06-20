#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <csignal>
#include <iostream>

char shar_object[] = "mem";
int shmid;

pid_t pid;

void ExitProgram(int sig) {
    kill(pid, SIGINT);
    exit(0);
}

void sys_err(char *msg) {
    puts(msg);
    exit(1);
}

int main() {
    (void) signal(SIGINT, ExitProgram);
    pid_t own_pid = getpid();

    std::cout << "My pid: " << own_pid << "\n";

    printf("Enter client pid: ");

    std::cin >> pid;

    if ((shmid = shm_open(shar_object, O_CREAT | O_RDWR, 0666)) == -1) {
        return -1;
    }

    int *msg_p = (int *) mmap(0, sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0);
    while (true) {
        sleep(1);
        printf("%d\n", *msg_p);
    }

    return 0;
}
