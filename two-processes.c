#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>



int main() {

    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, sizeof(int), 0666|IPC_CREAT);  

    if (shmid < 0){
        perror("shmget failed");
        return 1;
    }

    int *shared_number = (int*) shmat(shmid, NULL, 0);
    if (shared_number == (void*) -1) {
        perror("shmat failed");
        return 1;
    }

    *shared_number = 0;

    pid_t pid = fork();

    if (pid < 0){
        perror("fork failed");
        return 1;
    }
    
    for (int i = 0; i < 50; i++) {
        (*shared_number)++;
        if (pid == 0) {
            printf("Child: shared_number = %d\n", *shared_number);
        } else {
            printf("Parent: shared_number = %d\n", *shared_number);
        }
        usleep(1000);
    }

    shmdt(shared_number);
    if (pid > 0) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    
    return 0;
}