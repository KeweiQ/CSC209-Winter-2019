#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include "family.h"


void pt_familiy(Family* fam) {
    printf("***Family signature: %s Num words: %d\n", fam->signature, fam->num_words);
    for(int i = 0; i < fam->num_words; i++) {
        printf("     %s\n", fam->word_ptrs[i]);
    }
    printf("\n");
}


Family *set_up_family_list() {
    Family *fam1 = new_family("----");
    Family *fam2 = new_family("---");
    Family *fam3 = new_family("--");
    fam1->next = fam2;
    fam2->next = fam3;
    add_word_to_family(fam1, "e11");
    add_word_to_family(fam1, "e12");
    add_word_to_family(fam1, "e13");
    return fam1;
}


char **set_up_word_list() {
    char **list = malloc(sizeof(char*) * 10);
    list[0] = "ally";
    list[1] = "beta";
    list[2] = "cool";
    list[3] = "deal";
    list[4] = "else";
    list[5] = "flew";
    list[6] = "good";
    list[7] = "hope";
    list[8] = "ibex";
    list[9] = NULL;
    return list;
}


Family *test_new_family(char *str) {
    Family *new = new_family(str);
    pt_familiy(new);
    printf("%s\n", "1. test_new_family runs without error\n");
    return new;
}


void test_add_word_to_family(Family *fam) {
    add_word_to_family(fam, "e21");
    add_word_to_family(fam, "e22");
    add_word_to_family(fam, "e23");
    pt_familiy(fam);
    printf("%s\n", "2. test_add_word_to_family runs without error\n\n");
}


void test_find_family(Family *fam_list) {
    Family *result1 = find_family(fam_list, "---");
    Family *result2 = find_family(fam_list, "-");

    if (result1 == NULL) {
        printf("%s\n", "NULL");
    } else {
        pt_familiy(result1);
        printf("%s\n", "found!");
    }
    printf("%s", "3(1). test_find_family on existed fam runs without error\n");
    printf("%s", "\n");

    if (result2 == NULL) {
        printf("%s\n", "NULL\n");
    } else {
        pt_familiy(result2);
        printf("%s\n", "found!");
    }
    printf("%s", "3(2). test_find_family on doesn't existed fam runs without error\n");
    printf("%s", "\n");
}


void test_find_biggest_family(Family *fam_list) {
    Family *fam = find_biggest_family(fam_list);
    pt_familiy(fam);
    printf("%s\n", "4. test_find_biggest_family runs without error");
    printf("%s", "\n");
}


void test_deallocate_families(Family *fam_list) {
    deallocate_families(fam_list);
    printf("%s\n", "5. test_deallocate_families runs without error");
    printf("%s", "\n");
}


void test_generate_families(char **words, char letter) {
    Family *fam_list = generate_families(words, letter);
    print_families(fam_list);
    printf("%s\n", "6. test_generate_families runs without error");
    printf("%s", "\n");
}


void test_get_family_signature(Family *fam) {
    char *sig = get_family_signature(fam);
    printf("%s\n", sig);
    printf("%s\n", "7. test_get_family_signature runs without error");
    printf("%s", "\n");
}


void test_get_new_word_list(Family *fam) {
    char **new_list = get_new_word_list(fam);
    for(int i = 0; i < fam->num_words; i++) {
        printf("%s\n", fam->word_ptrs[i]);
    }
    uintptr_t add1 = (uintptr_t)(void *) fam->word_ptrs;
    printf("address of original list: %" PRIuPTR "\n", add1);
    for(int i = 0; i < fam->num_words; i++) {
        printf("%s\n", new_list[i]);
    }
    uintptr_t add2 = (uintptr_t)(void *) new_list;
    printf("address of original list: %" PRIuPTR "\n", add2);
    printf("%s\n", "8. test_get_new_word_list runs without error");
    printf("%s", "\n");
}


void test_get_random_word_from_family(Family *fam) {
    char *result1 = get_random_word_from_family(fam);
    char *result2 = get_random_word_from_family(fam);
    char *result3 = get_random_word_from_family(fam);
    char *result4 = get_random_word_from_family(fam);
    char *result5 = get_random_word_from_family(fam);
    char *result6 = get_random_word_from_family(fam);
    printf("%s\n", result1);
    printf("%s\n", result2);
    printf("%s\n", result3);
    printf("%s\n", result4);
    printf("%s\n", result5);
    printf("%s\n", result6);
    printf("%s\n", "9. test_get_random_word_from_family runs without error");
    printf("%s", "\n");
}


int main() {
    init_family(2);
    test_new_family("----");

    Family *list = set_up_family_list();
    test_add_word_to_family(list->next);
    test_find_family(list);
    test_find_biggest_family(list);
    test_deallocate_families(list);

    list = set_up_family_list();
    char **word_list = set_up_word_list();
    test_generate_families(word_list, 'e');
    test_get_family_signature(list);
    test_get_new_word_list(list);
    test_get_random_word_from_family(list);
    return 0;
}
