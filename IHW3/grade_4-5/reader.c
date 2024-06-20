#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void read_from_db(int index) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }
    
    sprintf(buffer, "reader %d", index);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
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
