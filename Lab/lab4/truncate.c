#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
    Write a function named truncate() that takes a string s and a
    non-negative integer n. If s has more than n characters (not including the
    null terminator), the function should truncate s at n characters and
    return the number of characters that were removed. If s has n or
    fewer characters, s is unchanged and the function returns 0. For example,
    if s is the string "function" and n is 3, then truncate() changes s to
    the string "fun" and returns 5.
*/
int truncate(char *target, int amt) {
    int result;
    if (amt < strlen(target)) {
        result = strlen(target) - amt;
    } else {
        result = 0;
    }
    strncpy(target, target, amt);
    // Question: why we can modify target even it is a read-only variable?
    target[amt] = '\0';
    return result;
}


int main(int argc, char **argv) {
    /* Do not change the main function */
    if (argc != 3) {
        fprintf(stderr, "Usage: truncate number string\n");
        exit(1);
    }
    int amt = strtol(argv[1], NULL, 10);

    // Isn't it a string literal?
    char *target = argv[2];

    int soln_val = truncate(target, amt);
    printf("%d %s\n", soln_val, target);

    return 0;
}
