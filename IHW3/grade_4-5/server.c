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

int database[DB_SIZE];
pthread_mutex_t db_mutex;
sem_t db_sem;

void initialize_database() {
    for (int i = 0; i < DB_SIZE; i++) {
        database[i] = i + 1;
    }
}

void handle_client(int client_socket) {
    char buffer[1024] = {0};
    int index, new_value, old_value;
    read(client_socket, buffer, 1024);
    
    char *token = strtok(buffer, " ");
    if (strcmp(token, "reader") == 0) {
        // Reader process
        token = strtok(NULL, " ");
        index = atoi(token);
        
        sem_wait(&db_sem);
        int value = database[index];
        sem_post(&db_sem);

        sprintf(buffer, "Reader: PID=%d, Index=%d, Value=%d, Fibonacci=%d\n", getpid(), index, value, fibonacci(value));
        send(client_socket, buffer, strlen(buffer), 0);
    } else if (strcmp(token, "writer") == 0) {
        // Writer process
        token = strtok(NULL, " ");
        index = atoi(token);
        token = strtok(NULL, " ");
        new_value = atoi(token);

        pthread_mutex_lock(&db_mutex);
        old_value = database[index];
        database[index] = new_value;
        // Re-sort the database
        qsort(database, DB_SIZE, sizeof(int), compare);
        pthread_mutex_unlock(&db_mutex);

        sprintf(buffer, "Writer: PID=%d, Index=%d, OldValue=%d, NewValue=%d\n", getpid(), index, old_value, new_value);
        send(client_socket, buffer, strlen(buffer), 0);
    }
    
    close(client_socket);
}

int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    initialize_database();
    pthread_mutex_init(&db_mutex, NULL);
    sem_init(&db_sem, 0, 1);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0) {
        if (fork() == 0) {
            close(server_fd);
            handle_client(new_socket);
            exit(0);
        }
        close(new_socket);
    }
    
    pthread_mutex_destroy(&db_mutex);
    sem_destroy(&db_sem);
    return 0;
}
