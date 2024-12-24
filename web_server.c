#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void handle_get(int client_socket, const char *path);
void handle_post(int client_socket, const char *body);
void send_response(int client_socket, const char *status, const char *content_type, const char *body);
void execute_cgi(int client_socket, const char *script_path, const char *body);

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d\n", PORT);

    while (1) {
        // Accept connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Receive request
        memset(buffer, 0, BUFFER_SIZE);
        read(client_socket, buffer, sizeof(buffer) - 1);

        // Parse HTTP request
        char method[8], path[256], protocol[16];
        sscanf(buffer, "%s %s %s", method, path, protocol);

        // Handle GET and POST methods
        if (strcmp(method, "GET") == 0) {
            handle_get(client_socket, path);
        } else if (strcmp(method, "POST") == 0) {
            char *body = strstr(buffer, "\r\n\r\n") + 4; // Skip headers
            handle_post(client_socket, body);
        } else {
            send_response(client_socket, "405 Method Not Allowed", "text/plain", "Only GET and POST are supported");
        }

        close(client_socket);
    }

    close(server_socket);
    return 0;
}

void handle_get(int client_socket, const char *path) {
    if (strcmp(path, "/") == 0) {
        path = "/index.html"; // Default file
    }

    char full_path[256];
    snprintf(full_path, sizeof(full_path), "www%s", path); // Files served from "www" directory

    FILE *file = fopen(full_path, "r");
    if (!file) {
        send_response(client_socket, "404 Not Found", "text/plain", "File not found");
        return;
    }

    char file_content[BUFFER_SIZE];
    fread(file_content, 1, sizeof(file_content) - 1, file);
    fclose(file);

    send_response(client_socket, "200 OK", "text/html", file_content);
}

void handle_post(int client_socket, const char *body) {
    const char *script_path = "cgi-bin/script.sh"; // CGI script location
    execute_cgi(client_socket, script_path, body);
}

void send_response(int client_socket, const char *status, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "\r\n"
             "%s",
             status, content_type, strlen(body), body);
    send(client_socket, response, strlen(response), 0);
}

void execute_cgi(int client_socket, const char *script_path, const char *body) {
    char command[BUFFER_SIZE];
    char output[BUFFER_SIZE];

    snprintf(command, sizeof(command), "echo \"%s\" | %s", body, script_path);

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        send_response(client_socket, "500 Internal Server Error", "text/plain", "Failed to execute CGI script");
        return;
    }

    fread(output, 1, sizeof(output) - 1, pipe);
    pclose(pipe);

    send_response(client_socket, "200 OK", "text/plain", output);
}