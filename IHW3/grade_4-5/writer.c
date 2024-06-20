#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void write_to_db(int index, int new_value) {
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
    
    sprintf(buffer, "writer %d %d", index, new_value);
    send(sock, buffer, strlen(buffer), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
    close(sock);
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <index> <new_value>\n", argv[0]);
        return -1;
    }

    int index = atoi(argv[1]);
    int new_value = atoi(argv[2]);
    write_to_db(index, new_value);
    return 0;
}
