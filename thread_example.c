#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// 쓰레드 함수
void *print_message(void *arg) {
    char *message = (char *)arg;
    for (int i = 0; i < 5; i++) {
        printf("%s\n", message);
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // 쓰레드 생성
    pthread_create(&thread1, NULL, print_message, "Hello from Thread 1");
    pthread_create(&thread2, NULL, print_message, "Hello from Thread 2");

    // 쓰레드 종료 대기
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Threads finished execution\n");
    return 0;
}