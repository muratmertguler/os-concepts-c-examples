#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
    int fd1[2]; // child -> parent
    int fd2[2]; // parent -> child
    char buffer[50];

    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("pipe failed");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        // ---- CHILD PROCESS ----
        close(fd1[0]); // child will not read from fd1
        close(fd2[1]); // child will not write to fd2

        int number = 1;
        while (number <= 5) {
            // send number to parent
            write(fd1[1], &number, sizeof(number));
            printf("Child sent: %d\n", number);

            // wait response from parent
            read(fd2[0], &number, sizeof(number));
            printf("Child received: %d\n", number);

            number++;
        }

        close(fd1[1]);
        close(fd2[0]);

    } else {
        // ---- PARENT PROCESS ----
        close(fd1[1]); // parent will not write to fd1
        close(fd2[0]); // parent will not read from fd2

        int number;
        while (1) {
            // wait number from child
            if (read(fd1[0], &number, sizeof(number)) <= 0) break;
            printf("Parent received: %d\n", number);

            number++; // increment and send back
            write(fd2[1], &number, sizeof(number));
            printf("Parent sent: %d\n", number);
        }

        close(fd1[0]);
        close(fd2[1]);
    }

    return 0;
}
