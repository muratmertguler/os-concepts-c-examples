#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

int *shared_number;
char* proc_type;  // Global değişken olarak tanımla

void* thread_function(void* arg) {
    int thread_id = *(int*)arg;

    for (int i = 0; i < 100; i++) {
        (*shared_number)++;
        printf("[%s] Thread %d: shared_number = %d\n", proc_type, thread_id, *shared_number);
        //usleep(1000);
    }

    return NULL;
}

int main() {
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }

    shared_number = (int*) shmat(shmid, NULL, 0);
    *shared_number = 0;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    // fork'tan sonra hangi process olduğumuzu burada belirliyoruz
    proc_type = (pid == 0) ? "Child" : "Parent";

    // Her iki process 2 thread oluşturur
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    pthread_create(&t1, NULL, thread_function, &id1);
    pthread_create(&t2, NULL, thread_function, &id2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    if (pid > 0) {
        wait(NULL);
        printf("[Parent] Final shared_number = %d\n", *shared_number);
        shmctl(shmid, IPC_RMID, NULL);
    }

    shmdt(shared_number);
    return 0;
}
