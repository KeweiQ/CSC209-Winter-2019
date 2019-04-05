#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "family.h"
#include "reading.h"


char **set_up() {
    char **list = malloc(sizeof(char*) * 6);
    list[0] = "python";
    list[1] = "java";
    list[2] = "c";
    list[3] = "hello";
    list[4] = "world";
    list[5] = NULL;
    return list;
}


void test_prune_word_list(char **words, int len) {
    int words_remaining = 0;
    char **list = prune_word_list(words, len, &words_remaining);
    int i = 0;
    printf("words_remaining: %d\n", words_remaining);
    while (list[i] != NULL) {
        printf("%s\n", list[i]);
        i++;
    }
    printf("%s\n", "1. test_prune_word_list runs without error\n");
}


void test_get_word_list_of_length(char **words) {
    int len;
    char **list = get_word_list_of_length(words, &len);
    int i = 0;
    while (list[i] != NULL) {
        printf("%s\n", list[i]);
        i++;
    }
    printf("%s\n", "2. test_get_word_list_of_length runs without error\n");
}


int main() {
    char **words = set_up();
    test_prune_word_list(words, 5);
    test_get_word_list_of_length(words);
}
