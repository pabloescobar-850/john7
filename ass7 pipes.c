//7. Pipes (CO5)
//Full duplex communication between parent and child processes.
//Parent process writes a pathname of a file (the contents of the file are desired) on one //pipe to be
//read by child process and child process writes the contents of the file on second pipe to //be read by
//parent process and displays on standard output.


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

int main()
{
    int pipe1[2], pipe2[2];
    pid_t pid;
    char filename[256];
    char buffer[1024];
    ssize_t n;

    // validation and process creation
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
    {
        perror("Pipe creation failed");
        return 1;
    }

    pid = fork();

    if (pid < 0)
    {
        perror("Fork failed");
        exit(0);
    }

    // parent process
    if (pid > 0)
    {
        // close
        close(pipe1[0]);
        close(pipe2[1]);

        // enter filename
        printf("Enter the file pathname \n");
        fgets(filename, sizeof(filename), stdin);
        filename[strcspn(filename, "\n")] = '\0';

        // write filename
        write(pipe1[1], filename, strlen(filename) + 1);
        close(pipe1[1]);

        // receive file content
        printf("Content received from child is:- \n");

        while ((n = read(pipe2[0], buffer, sizeof(buffer))) > 0)
        {
            write(STDOUT_FILENO, buffer, n);
        }

        close(pipe2[0]);
        wait(NULL);
    }
    else
    {
        // close
        close(pipe1[1]);
        close(pipe2[0]);

        // read filename
        n = read(pipe1[0], filename, sizeof(filename));
        close(pipe1[0]);

        // validate n
        if (n <= 0)
        {
            perror("Failed to receive a file");
            close(pipe2[1]);
            exit(1);
        }

        int fd = open(filename, O_RDONLY);

        if (fd == -1)
        {
            perror("Opening the file failed");
            close(pipe2[1]);
            exit(1);
        }

        // read file and send content to parent
        while ((n = read(fd, buffer, sizeof(buffer))) > 0)
        {
            if (write(pipe2[1], buffer, n) == -1)
            {
                perror("Write failed");
                close(fd);
                close(pipe2[1]);
                exit(1);
            }
        }

        close(fd);
        close(pipe2[1]);
        exit(0);
    }

    return 0;
}


// gcc program.c -o program
// ./program

// address.txt file banav
