//7. Pipes (CO5)
//Full duplex communication between parent and child processes.
//Parent process writes a pathname of a file (the contents of the file are desired) on one //pipe to be
//read by child process and child process writes the contents of the file on second pipe to //be read by
//parent process and displays on standard output.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main() {

    int pipe1[2], pipe2[2];  // Two pipes for communication
    pid_t pid;               // Stores process ID
    char filename[256];      // Stores file name
    char buffer[1024];       // Stores file contents
    ssize_t n;               // Number of bytes read

    // Create two pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    // Create child process using fork()
    pid = fork();

    // Check if fork failed
    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid > 0) {

        // PARENT PROCESS

        // Close unused pipe ends
        close(pipe1[0]);  // Close read end of pipe1
        close(pipe2[1]);  // Close write end of pipe2

        // Ask user to enter file path
        printf("Enter file pathname: ");
        fgets(filename, sizeof(filename), stdin);

        // Remove newline from filename
        filename[strcspn(filename, "\n")] = '\0';

        // Send filename to child through pipe1
        write(pipe1[1], filename, strlen(filename) + 1);

        // Close pipe1 after sending filename
        close(pipe1[1]);

        printf("\nFile contents received from child:\n\n");

        // Receive file contents from child through pipe2
        while ((n = read(pipe2[0], buffer, sizeof(buffer))) > 0) {

            // Display received contents on screen
            write(STDOUT_FILENO, buffer, n);
        }

        // Close pipe2
        close(pipe2[0]);

        // Wait for child process to finish
        wait(NULL);

    }

    else {

        // CHILD PROCESS

        // Close unused pipe ends
        close(pipe1[1]);  // Close write end of pipe1
        close(pipe2[0]);  // Close read end of pipe2

        // Receive filename from parent through pipe1
        n = read(pipe1[0], filename, sizeof(filename));

        // Close pipe1 after receiving filename
        close(pipe1[0]);

        // Check if filename was received
        if (n <= 0) {
            perror("Failed to receive filename");
            close(pipe2[1]);
            exit(1);
        }

        // Open the file in read-only mode
        int fd = open(filename, O_RDONLY);

        // Check if file opening failed
        if (fd == -1) {
            perror("File opening failed");
            close(pipe2[1]);
            exit(1);
        }

        // Read file contents
        while ((n = read(fd, buffer, sizeof(buffer))) > 0) {

            // Send file contents to parent through pipe2
            if (write(pipe2[1], buffer, n) == -1) {
                perror("Pipe write failed");

                // Close file and pipe if error occurs
                close(fd);
                close(pipe2[1]);
                exit(1);
            }
        }

        // Close file after reading
        close(fd);

        // Close pipe2 after sending data
        close(pipe2[1]);

        // End child process
        exit(0);
    }

    return 0;
}

