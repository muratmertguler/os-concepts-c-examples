#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

// Semaphore P (wait) işlemi
void sem_wait(int semid) {
    struct sembuf sb = {0, -1, 0};  
    semop(semid, &sb, 1);
}

// Semaphore V (signal) işlemi
void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};   
    semop(semid, &sb, 1);
}

int main() {
    // Shared memory ayarları
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT);

    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }

    int *shared_number = (int*) shmat(shmid, NULL, 0);
    if (shared_number == (void*) -1) {
        perror("shmat failed");
        return 1;
    }

    *shared_number = 0;

    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        return 1;
    }

    // Semaphore başlangıç değeri: 1 (binary semaphore - mutex gibi)
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl SETVAL failed");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    for (int i = 0; i < 50; i++) {
        sem_wait(semid);  // kilidi al
        (*shared_number)++;
        if (pid == 0) {
            printf("Child: shared_number = %d\n", *shared_number);
        } else {
            printf("Parent: shared_number = %d\n", *shared_number);
        }
        sem_signal(semid);  // kilidi bırak
        usleep(1000);
    }

    // Paylaşımlı bellekten ayrıl
    shmdt(shared_number);

    // Parent ise temizlik yap
    if (pid > 0) {
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);  // semaphore'u sil
    }

    return 0;
}
