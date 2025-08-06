#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>  // O_CREAT için

int *shared_number;
pthread_mutex_t *mutex;  // Shared memory üzerinden mutex

char* proc_type;  // "Parent" / "Child" bilgisi

void* thread_function(void* arg) {
    int thread_id = *(int*)arg;

    for (int i = 0; i < 100; i++) {
        pthread_mutex_lock(mutex);  
        (*shared_number)++;
        printf("[%s] Thread %d: shared_number = %d\n", proc_type, thread_id, *shared_number);
        pthread_mutex_unlock(mutex); 
        //usleep(1000); 
    }

    return NULL;
}

int main() {
    // Ortak bellek oluştur
    key_t key_num = ftok("shmfile", 65);
    key_t key_mutex = ftok("shmfile", 75);  // Mutex için farklı key

    int shmid_num = shmget(key_num, sizeof(int), 0666 | IPC_CREAT);
    int shmid_mutex = shmget(key_mutex, 1024, 0666 | IPC_CREAT);


    if (shmid_num < 0 || shmid_mutex < 0) {
        perror("shmget failed");
        return 1;
    }

    shared_number = (int*) shmat(shmid_num, NULL, 0);
    mutex = (pthread_mutex_t*) shmat(shmid_mutex, NULL, 0);

    *shared_number = 0;

    // Sadece parent mutex’i init eder
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        proc_type = "Child";
    } else {
        proc_type = "Parent";
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(mutex, &attr);
    }

    // Her process 2 thread oluşturur
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    pthread_create(&t1, NULL, thread_function, &id1);
    pthread_create(&t2, NULL, thread_function, &id2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    if (pid > 0) {
        wait(NULL);
        printf("[Parent] Final shared_number = %d\n", *shared_number);

        // Temizlik
        pthread_mutex_destroy(mutex);
        shmctl(shmid_num, IPC_RMID, NULL);
        shmctl(shmid_mutex, IPC_RMID, NULL);
    }

    // Her iki process shared memory’den ayrılır
    shmdt(shared_number);
    shmdt(mutex);

    return 0;
}
