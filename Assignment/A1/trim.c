#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Reads a trace file produced by valgrind and an address marker file produced
 * by the program being traced. Outputs only the memory reference lines in
 * between the two markers
 */

int main(int argc, char **argv) {

    if(argc != 3) {
         fprintf(stderr, "Usage: %s tracefile markerfile\n", argv[0]);
         exit(1);
    }

    // read marker file and store starting & ending address
    FILE *fp1;
    fp1 = fopen(argv[2], "r");
    long start_marker, end_marker;
    fscanf(fp1, "%lx %lx", &start_marker, &end_marker);
    fclose(fp1);

    // read trace file
    FILE *fp2;
    fp2 = fopen(argv[1], "r");
    int print = 0;
    char type;
    long addr;
    int byte;

    int r = fscanf(fp2, "%c %lx,%d\n", &type, &addr, &byte);
    while (r != EOF) {
        if (addr == end_marker) {
            print = 0;
        }
        if (print == 1) {
            printf("%c,%#lx\n", type, addr);
        }
        if (addr == start_marker) {
            print = 1;
        }
        r = fscanf(fp2, "%c %lx,%d\n", &type, &addr, &byte);
    }
    fclose(fp2);

    return 0;
}
