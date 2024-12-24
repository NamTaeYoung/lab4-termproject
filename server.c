#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char username[BUFFER_SIZE];
} client_t;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char *message, int exclude_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->socket != exclude_socket) {
            send(clients[i]->socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void handle_file_transfer(int client_socket, const char *filename) {
    FILE *file = fopen(filename, "rb");
    char buffer[BUFFER_SIZE];

    if (file == NULL) {
        snprintf(buffer, sizeof(buffer), "ERROR: File %s not found.\n", filename);
        send(client_socket, buffer, strlen(buffer), 0);
        return;
    }

    snprintf(buffer, sizeof(buffer), "START_FILE %s\n", filename);
    send(client_socket, buffer, strlen(buffer), 0);

    while (!feof(file)) {
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
        send(client_socket, buffer, bytes_read, 0);
    }

    strcpy(buffer, "END_FILE\n");
    send(client_socket, buffer, strlen(buffer), 0);

    fclose(file);
}

void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    if (recv(client->socket, client->username, sizeof(client->username), 0) <= 0) {
        close(client->socket);
        free(client);
        pthread_exit(NULL);
    }

    snprintf(message, sizeof(message), "%s has joined the chat\n", client->username);
    printf("%s", message);
    broadcast_message(message, client->socket);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client->socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            snprintf(message, sizeof(message), "%s has left the chat\n", client->username);
            printf("%s", message);
            broadcast_message(message, client->socket);

            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == client) {
                    clients[i] = NULL;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);

            close(client->socket);
            free(client);
            pthread_exit(NULL);
        }

        if (strncmp(buffer, "!sendfile", 9) == 0) {
            char *filename = buffer + 10;
            handle_file_transfer(client->socket, filename);
        } else {
            snprintf(message, sizeof(message), "%s: %s", client->username, buffer);
            printf("%s", message);
            broadcast_message(message, client->socket);
        }
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    memset(clients, 0, sizeof(clients));

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, MAX_CLIENTS);

    printf("Server started on port 8080\n");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        client_t *new_client = (client_t *)malloc(sizeof(client_t));
        new_client->socket = client_socket;

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == NULL) {
                clients[i] = new_client;
                pthread_t thread_id;
                pthread_create(&thread_id, NULL, handle_client, new_client);
                pthread_detach(thread_id);
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    return 0;
}