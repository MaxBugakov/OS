#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int sock = 0;

void handle_sigint(int sig) {
    if (sock > 0) {
        close(sock);
    }
    printf("Observer disconnected\n");
    exit(0);
}

void observe() {
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    sprintf(buffer, "observer");
    sendto(sock, buffer, strlen(buffer), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    while (1) {
        int n = recvfrom(sock, buffer, BUFFER_SIZE, 0, NULL, NULL);
        buffer[n] = '\0';
        printf("%s", buffer);
    }
    
    close(sock);
}

int main(int argc, char const *argv[]) {
    signal(SIGINT, handle_sigint);
    observe();
    return 0;
}
