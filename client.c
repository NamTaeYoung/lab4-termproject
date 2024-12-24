#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void *receive_messages(void *arg) {
    int socket = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            printf("Disconnected from server\n");
            exit(0);
        }

        printf("%s", buffer);
    }
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char username[BUFFER_SIZE];

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with server IP if needed
    server_addr.sin_port = htons(8080);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    send(client_socket, username, strlen(username), 0);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, &client_socket);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, sizeof(buffer), stdin);

        if (strncmp(buffer, "/exit", 5) == 0) {
            printf("Exiting chat...\n");
            close(client_socket);
            exit(0);
        }

        send(client_socket, buffer, strlen(buffer), 0);
    }

    return 0;
}