#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void read_from_db(int index) {
    int sock;
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
    
    sprintf(buffer, "reader %d", index);
    sendto(sock, buffer, strlen(buffer), 0, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    int n = recvfrom(sock, buffer, BUFFER_SIZE, 0, NULL, NULL);
    buffer[n] = '\0';
    printf("%s", buffer);
    
    close(sock);
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <index>\n", argv[0]);
        return -1;
    }

    int index = atoi(argv[1]);
    read_from_db(index);
    return 0;
}
