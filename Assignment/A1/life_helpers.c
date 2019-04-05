#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_state(char *state, int size) {
    printf("%s\n", state);
}


void update_state(char *state, int size) {
    char new[size + 1];
    new[0] = state[0];
    new[size - 1] = state[size - 1];
    new[size] = '\0';

    for(int i = 0; i < size - 2; i++) {
        if (state[i] != state[i + 2]) {
            new[i + 1] = 'X';
        } else {
            new[i + 1] = '.';
        }
    }

    strcpy(state, new);
}
