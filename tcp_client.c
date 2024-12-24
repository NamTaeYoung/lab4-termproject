#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // 서버 주소 설정
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // 서버 주소 변환
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        return -1;
    }

    // 서버 연결
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to server. Type messages to send:\n");

    while (1) {
        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character

        if (strcmp(buffer, "/exit") == 0) {
            printf("Exiting...\n");
            break;
        }

        // 서버에 메시지 전송
        send(sock, buffer, strlen(buffer), 0);

        // 서버로부터 응답 받기
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(sock, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            printf("Server response: %s\n", buffer);
        }
    }

    close(sock);
    return 0;
}