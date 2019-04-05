#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Constants that determine that address ranges of different memory regions

#define GLOBALS_START 0x400000
#define GLOBALS_END   0x700000
#define HEAP_START   0x4000000
#define HEAP_END     0x8000000
#define STACK_START 0xfff000000

int main(int argc, char **argv) {

    FILE *fp = NULL;

    if(argc == 1) {
        fp = stdin;

    } else if(argc == 2) {
        fp = fopen(argv[1], "r");
        if(fp == NULL){
            perror("fopen");
            exit(1);
        }
    } else {
        fprintf(stderr, "Usage: %s [tracefile]\n", argv[0]);
        exit(1);
    }

    // variables to count different values
    int ins = 0;
    int mod = 0;
    int load = 0;
    int store = 0;

    int global = 0;
    int heap = 0;
    int stack = 0;

    // scan trace file and record values
    char type;
    long addr;
    int r = fscanf(fp, "%c,%lx\n", &type, &addr);
    while (r != EOF) {
        // check type
        if (type == 'I') {
            ins++;
        } else if (type == 'M') {
            mod++;
        } else if (type == 'L') {
            load++;
        } else if (type == 'S'){
            store++;
        }
        // check address
        if ((addr >= GLOBALS_START) && (addr <= GLOBALS_END) && (type != 'I')) {
            global++;
        } else if ((addr >= HEAP_START) && (addr <= HEAP_END) && (type != 'I')) {
            heap++;
        } else if (addr >= STACK_START && type != 'I') {
            stack++;
        }
        //scan next line
        r = fscanf(fp, "%c,%lx\n", &type, &addr);
    }
    fclose(fp);

    // print output
    printf("Reference Counts by Type:\n");
    printf("    Instructions: %d\n", ins);
    printf("    Modifications: %d\n", mod);
    printf("    Loads: %d\n", load);
    printf("    Stores: %d\n", store);
    printf("Data Reference Counts by Location:\n");
    printf("    Globals: %d\n", global);
    printf("    Heap: %d\n", heap);
    printf("    Stack: %d\n", stack);

    return 0;
}
