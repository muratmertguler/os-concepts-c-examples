#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int shared_number = 0;
pthread_mutex_t lock;

void *thread_1(void* arg){
    int* num_ptr = (int*)arg;

    while(1) {
        pthread_mutex_lock(&lock);
        if (*num_ptr > 100){
            pthread_mutex_unlock(&lock);
            break;
        }
        *num_ptr += 1;
        printf("Thread 1: shared_number = %d\n", *num_ptr);
        pthread_mutex_unlock(&lock);
        usleep(2000);
    }
    return NULL;
}

void *thread_2(void* arg){
    int* num_ptr = (int*)arg;

    while(1) {
        pthread_mutex_lock(&lock);
        if (*num_ptr > 100){
            pthread_mutex_unlock(&lock);
            break;
        }
        *num_ptr += 1;
        printf("Thread 2: shared_number = %d\n", *num_ptr);
        pthread_mutex_unlock(&lock);
        usleep(2000);
        
    }
    return NULL;
}

int main() {

    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_1, &shared_number);
    pthread_create(&t2, NULL, thread_2, &shared_number);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Final shared_number = %d \n", shared_number);

    pthread_mutex_destroy(&lock);

    return 0;
}