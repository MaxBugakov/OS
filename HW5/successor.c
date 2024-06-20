#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int received_number = 0;
int bit_count = 0;

void receive_bit(int sig) {
    if (sig == SIGUSR1) {
        // Приняли бит 0
        printf("Received bit 0\n");
    } else {
        // Приняли бит 1
        received_number |= (1 << bit_count);
        printf("Received bit 1\n");
    }

    bit_count++;

    if (bit_count == 32) {
        printf("Received number: %d\n", received_number);
        bit_count = 0;
        received_number = 0;
        kill(getppid(), SIGUSR1); // Подтверждаем завершение передачи
    } else {
        kill(getppid(), sig); // Подтвержаем принятие бита
    }
}

int main() {
    printf("My PID: %d\n", getpid());

    signal(SIGUSR1, receive_bit);
    signal(SIGUSR2, receive_bit);

    while (1) {
        // Ждем сигналов
    }

    return 0;
}