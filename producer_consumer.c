#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int count = 0;
pthread_mutex_t mutex;
pthread_cond_t cond_producer, cond_consumer;

// 생산자 함수
void *producer(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&cond_producer, &mutex);
        }

        // 생산 작업
        int item = rand() % 100;
        buffer[count++] = item;
        printf("Produced: %d\n", item);

        pthread_cond_signal(&cond_consumer);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
}

// 소비자 함수
void *consumer(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&cond_consumer, &mutex);
        }

        // 소비 작업
        int item = buffer[--count];
        printf("Consumed: %d\n", item);

        pthread_cond_signal(&cond_producer);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
}

int main() {
    pthread_t producers[2], consumers[2];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_producer, NULL);
    pthread_cond_init(&cond_consumer, NULL);

    // 생산자와 소비자 생성
    for (int i = 0; i < 2; i++) {
        pthread_create(&producers[i], NULL, producer, NULL);
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_producer);
    pthread_cond_destroy(&cond_consumer);

    return 0;
}