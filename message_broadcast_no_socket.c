#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int id;
    char message[BUFFER_SIZE];
    int has_message;
} client_t;

client_t clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t message_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER; // 입력 동기화용 뮤텍스

int client_count = 0;

void *client_thread(void *arg) {
    int id = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    while (1) {
        pthread_mutex_lock(&input_mutex); // 입력 동기화 시작
        printf("[Client %d] Enter a message (type '/exit' to quit): ", id);
        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
        pthread_mutex_unlock(&input_mutex); // 입력 동기화 종료

        if (strcmp(buffer, "/exit") == 0) {
            printf("[Client %d] Exiting...\n", id);
            pthread_mutex_lock(&clients_mutex);
            snprintf(clients[id].message, sizeof(clients[id].message), "Client %d has left the chat.", id);
            clients[id].has_message = -1; // Mark client as exited
            pthread_mutex_unlock(&clients_mutex);
            pthread_cond_signal(&message_cond);
            break;
        }

        if (strlen(buffer) > 0) { // Only process non-empty messages
            pthread_mutex_lock(&clients_mutex);
            strncpy(clients[id].message, buffer, sizeof(buffer));
            clients[id].has_message = 1;
            pthread_mutex_unlock(&clients_mutex);

            pthread_cond_signal(&message_cond);
        }
    }

    return NULL;
}

void *server_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&clients_mutex);

        // Wait for a message from any client
        pthread_cond_wait(&message_cond, &clients_mutex);

        // Broadcast the message to all clients
        for (int i = 0; i < client_count; i++) {
            if (clients[i].has_message == 1) {
                printf("[Server] Broadcasting from Client %d: %s\n", i, clients[i].message);

                for (int j = 0; j < client_count; j++) {
                    if (j != i && clients[j].has_message != -1) {
                        printf("[Server -> Client %d] %s\n", j, clients[i].message);
                    }
                }

                clients[i].has_message = 0; // Mark message as processed
            } else if (clients[i].has_message == -1) { // Handle client exit
                printf("[Server] Client %d has disconnected.\n", i);

                for (int j = 0; j < client_count; j++) {
                    if (j != i && clients[j].has_message != -1) {
                        printf("[Server -> Client %d] Client %d has left the chat.\n", j, i);
                    }
                }

                clients[i].has_message = 0; // Mark as processed
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    return NULL;
}

int main() {
    pthread_t server_tid, client_tids[MAX_CLIENTS];

    // Initialize clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].id = i;
        clients[i].has_message = 0;
    }

    // Start server thread
    pthread_create(&server_tid, NULL, server_thread, NULL);

    // Create client threads
    printf("Enter number of clients to create (max %d): ", MAX_CLIENTS);
    scanf("%d", &client_count);
    getchar(); // Consume newline

    if (client_count > MAX_CLIENTS || client_count <= 0) {
        printf("Invalid number of clients. Exiting...\n");
        return 1;
    }

    for (int i = 0; i < client_count; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&client_tids[i], NULL, client_thread, id);
    }

    // Wait for client threads to finish
    for (int i = 0; i < client_count; i++) {
        pthread_join(client_tids[i], NULL);
    }

    printf("All clients have exited. Shutting down server...\n");
    pthread_cancel(server_tid);
    pthread_join(server_tid, NULL);

    return 0;
}