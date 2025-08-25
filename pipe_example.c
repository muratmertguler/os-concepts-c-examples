#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
    int fd1[2];  // Pipe for Parent -> Child
    int fd2[2];  // Pipe for Child -> Parent
    char buffer[100];

    // Create pipes
    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("pipe failed");
        exit(1);
    }

    // Fork to create child process
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        // ---- CHILD PROCESS ----
        close(fd1[1]);  // Child does not write to fd1
        close(fd2[0]);  // Child does not read from fd2

        // Read message from Parent
        read(fd1[0], buffer, sizeof(buffer));
        printf("Child received: %s\n", buffer);

        // Send reply to Parent
        char reply[] = "Hello Parent, I got your message!";
        write(fd2[1], reply, strlen(reply) + 1);

        // Close pipe ends
        close(fd1[0]);
        close(fd2[1]);

    } else {
        // ---- PARENT PROCESS ----
        close(fd1[0]);  // Parent does not read from fd1
        close(fd2[1]);  // Parent does not write to fd2

        // Send message to Child
        char msg[] = "Hello Child, how are you?";
        write(fd1[1], msg, strlen(msg) + 1);

        // Read reply from Child
        read(fd2[0], buffer, sizeof(buffer));
        printf("Parent received: %s\n", buffer);

        // Close pipe ends
        close(fd1[1]);
        close(fd2[0]);
    }

    return 0;
}
