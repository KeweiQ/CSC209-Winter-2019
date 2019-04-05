#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
    char user_id[MAXLINE];
    char password[MAXLINE];

    if(fgets(user_id, MAXLINE, stdin) == NULL) {
        perror("fgets");
        exit(1);
    }
    if(fgets(password, MAXLINE, stdin) == NULL) {
        perror("fgets");
        exit(1);
    }

    int n, status;
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    n = fork();
    if (n == -1) {
        perror("fork");
        exit(1);
    } else if (n == 0) {
        if (close(fd[1]) == -1) {
            perror("close writing end in child");
            exit(1);
        }

        int dupstatus = dup2(fd[0], STDIN_FILENO);
        if (dupstatus == -1) {
            perror("dup2");
            exit(1);
        }
        execl("./validate", "validate", NULL);

        if (close(fd[0]) == -1) {
            perror("close reading end in child");
            exit(1);
        }
    } else {
        if (close(fd[0]) == -1) {
            perror("close reading end in parent");
            exit(1);
        }

        if(write(fd[1], user_id, MAX_PASSWORD) == -1) {
            perror("write");
            exit(1);
        }
        if(write(fd[1], password, MAX_PASSWORD) == -1) {
            perror("write");
            exit(1);
        }

        if (close(fd[1]) == -1) {
            perror("close writing end in parent");
            exit(1);
        }

        if (wait(&status) == -1) {
            perror("wait");
            exit(1);
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                printf("%s\n", SUCCESS);
            } else if (WEXITSTATUS(status) == 2) {
                printf("%s\n", INVALID);
            } else if (WEXITSTATUS(status) == 3) {
                printf("%s\n", NO_USER);
            }
        }
    }


  return 0;
}
