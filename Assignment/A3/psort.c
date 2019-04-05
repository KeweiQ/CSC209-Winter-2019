#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "helper.h"


void sort(char *input_file, int **fd, int idx_child, int quotient, int remainder) {
    // open input file as read mode
    FILE *fp_input = fopen(input_file, "rb");
    if (fp_input == NULL) {
        perror("fopen while opening input file");
        exit(1);
    }

    // calculate the starting point of reading operaton
    int position, each;
    if (idx_child < remainder) {
        each = quotient + 1;
        position = (quotient + 1) * idx_child;
    } else {
        each = quotient;
        position = (quotient + 1) * remainder + quotient * (idx_child - remainder);
    }

    // create an array and read elements from input file into this array
    struct rec arr[each];
    if (fseek(fp_input, position * sizeof(struct rec), SEEK_SET) == -1) {
        perror("fseek");
        exit(1);
    }
    for (int idx_read = 0; idx_read < each; idx_read++) {
        if (fread(&(arr[idx_read]), sizeof(struct rec), 1, fp_input) == -1) {
            fprintf(stderr, "fread");
            exit(1);
        }
    }

    // sort the array
    qsort(arr, each, sizeof(struct rec), compare_freq);

    // write elementes in sorted array into the pipe one by one
    for (int idx_write = 0; idx_write < each; idx_write++) {
        if (write(fd[idx_child][1], &(arr[idx_write]), sizeof(struct rec)) == -1) {
            perror("write");
            exit(1);
        }
    }

    // close input file
    if (fclose(fp_input) == -1) {
        perror("fclose while closing input file");
        exit(1);
    }
}


void merge(char *output_file, int **fd, int num_children, int total) {
    // open output file
    FILE *fp_output;
    if ((fp_output = fopen(output_file, "wb")) == NULL) {
        perror("fopen while opening output file");
        exit(1);
    }

    // create an sorting array and fill it using the first element of each pipe
    int finished = 0;
    struct rec wait_sort[num_children];
    for (int idx_fill = 0; idx_fill < num_children; idx_fill++) {
        if (read(fd[idx_fill][0], &(wait_sort[idx_fill]), sizeof(struct rec)) == -1) {
            perror("read while filling");
            exit(1);
        }
    }

    // merge sorted arrays into a whole sorted array
    while (finished != total) {
        // find the smallest element in sorting array
        int min_idx;
        struct rec min_data;
        for (int i = num_children - 1; i >= 0; i--) {
            if (wait_sort[i].freq != -1) {
                min_data = wait_sort[i];
                min_idx = i;
            }
        }

        int idx_sort;
        for (idx_sort = 0; idx_sort < num_children; idx_sort++) {
            if ((wait_sort[idx_sort].freq != -1) && (compare_freq(&(wait_sort[idx_sort]), &min_data) == -1)) {
                min_data = wait_sort[idx_sort];
                min_idx = idx_sort;
            }
        }

        // write the smallest element to output file
        if (fwrite(&min_data, sizeof(struct rec), 1, fp_output) == 0) {
            fprintf(stderr, "fwrite");
        }

        // read next element from pipe
        if (read(fd[min_idx][0], &(wait_sort[min_idx]), sizeof(struct rec)) == 0) {
            wait_sort[min_idx].freq = -1;
        }

        finished++;
    }

    // close output file
    if (fclose(fp_output) == -1) {
        perror("fclose while closing output file");
        exit(1);
    }
}


int main(int argc, char* argv[]) {
    // check number of command-line arguments
    if (argc != 7) {
        fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
    }

    // check form of command-line arguments and get information from them
    int input_num;
    char *input_file;
    char *output_file;
    char opt;

    while ((opt = getopt(argc, argv, "n:f:o:")) != -1) {
        switch (opt) {
            case 'n':
                input_num = strtol(optarg, NULL, 10);
                break;
            case 'f':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
                exit(1);
        }
    }

    // check if the input number is valid and calculate the number of elements each child should read
    int total = get_file_size(input_file) / sizeof(struct rec);

    int num_children;
    if (input_num <= 0) {
        num_children = 1;
    } else if (input_num > total) {
        num_children = total;
    } else {
        num_children = input_num;
    }

    int quotient = total / num_children;
    int remainder = total - (quotient * num_children);

    // create pipe
    int **fd = malloc(sizeof(int *) * num_children);
    for (int idx_pipe = 0; idx_pipe < num_children; idx_pipe++) {
        fd[idx_pipe] = malloc(sizeof(int) * 2);
        if (pipe(fd[idx_pipe]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    // create child processes
    for (int idx_child = 0; idx_child < num_children; idx_child++) {
        int n = fork();
        if (n == -1) {
            perror("fork");
            exit(1);
        } else if (n == 0) { // child process sort 1/N elements from input file
            // close the reading end of pipe for child process
            for (int idx_cc = 0; idx_cc <= idx_child; idx_cc++) {
                if (close(fd[idx_cc][0]) == -1) {
                    perror("close reading end in child");
                    exit(1);
                }
            }

            // read 1/N elements from input file, sort them and write the result into pipe
	          sort(input_file, fd, idx_child, quotient, remainder);

            // close the writing end of pipe for child process
            if (close(fd[idx_child][1]) == -1) {
                perror("close writing end in child");
                exit(1);
            }

            // free dynamically allocated memory for pipe in child process
            for (int idx_pipe = 0; idx_pipe < num_children; idx_pipe++) {
                free(fd[idx_pipe]);
            }
            free(fd);

            exit(0);

        } else { // parent process
            // close the writing end of pipe for parent process
            if (close(fd[idx_child][1]) == -1) {
                perror("close writing end in parent");
                exit(1);
            }
        }
    }

    // merge sorted array of each child
    merge(output_file, fd, num_children, total);

    // close the reading end of pipe for parent process
    for (int idx_cp = 0; idx_cp < num_children; idx_cp++) {
        if (close(fd[idx_cp][0]) == -1) {
            perror("close reading end in parent");
            exit(1);
        }
    }

    // parent wait for all children terminated
    int status;
    for (int idx_wait = 0; idx_wait < num_children; idx_wait++) {
        if (wait(&status) == -1) {
            perror("wait");
            exit(1);
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 1) {
                fprintf(stderr, "Child terminated abnormally\n");
            }
        }
    }

    // free dynamically allocated memory for pipe in parent process
    for (int idx_pipe = 0; idx_pipe < num_children; idx_pipe++) {
        free(fd[idx_pipe]);
    }
    free(fd);

    return 0;
}
