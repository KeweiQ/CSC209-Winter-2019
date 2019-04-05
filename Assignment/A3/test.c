#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.h"


int main(int argc, char *argv[]){
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    int total = get_file_size(argv[1]) / sizeof(struct rec);
    int print_num;
    if (total <= 100) {
        print_num = total;
    } else {
        print_num = 100;
    }

    struct rec *t = malloc(sizeof(struct rec));
    for (int i = 0; i < print_num; i ++) {
        fread(t, sizeof(struct rec), 1, fp);
        printf("%d, %s\n", t->freq, t->word);
    }

    if (fclose(fp) == -1) {
        perror("fclose");
        exit(1);
    }
    
    free(t);
}
