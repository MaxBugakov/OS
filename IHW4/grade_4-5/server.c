#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <semaphore.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define DB_SIZE 100
#define BUFFER_SIZE 1024

int database[DB_SIZE];
pthread_mutex_t db_mutex;
sem_t db_sem;

void initialize_database() {
    for (int i = 0; i < DB_SIZE; i++) {
        database[i] = i + 1;
    }
}

int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void handle_client(char *buffer, struct sockaddr_in *client_addr, int sock) {
    char response[BUFFER_SIZE];
    int index, new_value, old_value;
    char *token = strtok(buffer, " ");
    
    if (strcmp(token, "reader") == 0) {
        token = strtok(NULL, " ");
        index = atoi(token);
        
        sem_wait(&db_sem);
        int value = database[index];
        sem_post(&db_sem);

        sprintf(response, "Reader: PID=%d, Index=%d, Value=%d, Fibonacci=%d\n", getpid(), index, value, fibonacci(value));
        sendto(sock, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
    } else if (strcmp(token, "writer") == 0) {
        token = strtok(NULL, " ");
        index = atoi(token);
        token = strtok(NULL, " ");
        new_value = atoi(token);

        pthread_mutex_lock(&db_mutex);
        old_value = database[index];
        database[index] = new_value;
        qsort(database, DB_SIZE, sizeof(int), compare);
        pthread_mutex_unlock(&db_mutex);

        sprintf(response, "Writer: PID=%d, Index=%d, OldValue=%d, NewValue=%d\n", getpid(), index, old_value, new_value);
        sendto(sock, response, strlen(response), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
    }
}

int main(int argc, char const *argv[]) {
    int sock;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    initialize_database();
    pthread_mutex_init(&db_mutex, NULL);
    sem_init(&db_sem, 0, 1);
    
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(sock, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        int n = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        buffer[n] = '\0';
        handle_client(buffer, &client_addr, sock);
    }
    
    pthread_mutex_destroy(&db_mutex);
    sem_destroy(&db_sem);
    close(sock);
    return 0;
}
