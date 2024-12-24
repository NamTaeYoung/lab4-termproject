#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char username[BUFFER_SIZE];

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server!\n");
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    // Send username to server
    send(client_socket, username, strlen(username), 0);

    // Create a child process for receiving messages
    if (fork() == 0) {
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int valread = read(client_socket, buffer, BUFFER_SIZE);
            if (valread > 0) {
                printf("%s\n", buffer);
            }
        }
    }

    // Parent process for sending messages
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        send(client_socket, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "/exit") == 0) {
            printf("Exiting...\n");
            close(client_socket);
            exit(0);
        }
    }

    return 0;
}