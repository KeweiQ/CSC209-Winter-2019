#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "family.h"

/* Number of word pointers allocated for a new family.
   This is also the number of word pointers added to a family
   using realloc, when the family is full.
*/
static int family_increment = 0;


/* Set family_increment to size, and initialize random number generator.
   The random number generator is used to select a random word from a family.
   This function should be called exactly once, on startup.
*/
void init_family(int size) {
    family_increment = size;
    srand(time(0));
}


/* Given a pointer to the head of a linked list of Family nodes,
   print each family's signature and words.

   Do not modify this function. It will be used for marking.
*/
void print_families(Family* fam_list) {
    int i;
    Family *fam = fam_list;

    while (fam) {
        printf("***Family signature: %s Num words: %d\n",
               fam->signature, fam->num_words);
        for(i = 0; i < fam->num_words; i++) {
            printf("     %s\n", fam->word_ptrs[i]);
        }
        printf("\n");
        fam = fam->next;
    }
}


/* Return a pointer to a new family whose signature is
   a copy of str. Initialize word_ptrs to point to
   family_increment+1 pointers, numwords to 0,
   maxwords to family_increment, and next to NULL.
*/
Family *new_family(char *str) {
    Family *new = malloc(sizeof(Family));
    if (new == NULL) {
        perror("malloc on family");
        exit(1);
    }
    
    new->signature = malloc(sizeof(char) * (strlen(str) + 1));
    if (new->signature == NULL) {
        perror("malloc on signature");
        exit(1);
    }
    strcpy(new->signature, str);
    new->signature[strlen(str)] = '\0';

    new->word_ptrs = malloc(sizeof(char*) * (family_increment + 1));
    if (new->word_ptrs == NULL) {
        perror("malloc on word_ptrs");
        exit(1);
    }

    new->num_words = 0;
    new->max_words = family_increment;
    new->next = NULL;
    new->word_ptrs[0] = NULL;

    return new;
}


/* Add word to the next free slot fam->word_ptrs.
   If fam->word_ptrs is full, first use realloc to allocate family_increment
   more pointers and then add the new pointer.
*/
void add_word_to_family(Family *fam, char *word) {
    if (fam->num_words == fam->max_words) {
        fam->word_ptrs = realloc(fam->word_ptrs, sizeof(char*) * (fam->max_words + family_increment + 1));
        fam->max_words += family_increment;
    }
    if (fam->word_ptrs == NULL) {
        perror("realloc on add word");
        exit(1);
    }

    fam->word_ptrs[fam->num_words] = word;
    fam->word_ptrs[fam->num_words + 1] = NULL;
    fam->num_words++;
}


/* Return a pointer to the family whose signature is sig;
   if there is no such family, return NULL.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_family(Family *fam_list, char *sig) {
    Family *fam = fam_list;
    while (fam != NULL) {
        if (strcmp(fam->signature, sig) == 0) {
            return fam;
        } else {
            fam = fam->next;
        }
    }
    return NULL;
}


/* Return a pointer to the family in the list with the most words;
   if the list is empty, return NULL. If multiple families have the most words,
   return a pointer to any of them.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_biggest_family(Family *fam_list) {
    if (fam_list == NULL) {
        return NULL;
    }

    Family *biggest = fam_list;
    Family *current = fam_list->next;

    while (current != NULL) {
        if (current->num_words > biggest->num_words) {
            biggest = current;
        }
        current = current->next;
    }

    return biggest;
}


/* Deallocate all memory rooted in the List pointed to by fam_list. */
void deallocate_families(Family *fam_list) {
    Family *current = fam_list;
    Family *next;

    if (current != NULL) {
        while (current != NULL) {
            free(current->signature);
            free(current->word_ptrs);
            next = current->next;
            free(current);
            current = next;
        }
    }
}


/* Generate and return a linked list of all families using words pointed to
   by word_list, using letter to partition the words.

   Implementation tips: To decide the family in which each word belongs, you
   will need to generate the signature of each word. Create only the families
   that have at least one word from the current word_list.
*/
Family *generate_families(char **word_list, char letter) {
    int length = strlen(word_list[0]);
    char sig[length + 1];
    Family *fam_list = NULL;
    int wl = 0;

    while (word_list[wl] != NULL) {
        // get signature of each word
        for (int i = 0; i < length; i++) {
            if (word_list[wl][i] == letter) {
                sig[i] = letter;
            } else {
                sig[i] = '-';
            }
        }
	sig[length] = '\0';

        // check whether word's signature already in one family and add it
        Family *existed_fam = find_family(fam_list, sig);
        if (existed_fam == NULL) {
            Family *new_fam = new_family(sig);
            add_word_to_family(new_fam, word_list[wl]);
            new_fam->next = fam_list;
            fam_list = new_fam;
        } else {
            add_word_to_family(existed_fam, word_list[wl]);
        }
        wl++;
    }

    return fam_list;
}


/* Return the signature of the family pointed to by fam. */
char *get_family_signature(Family *fam) {
    return fam->signature;
}


/* Return a pointer to word pointers, each of which
   points to a word in fam. These pointers should not be the same
   as those used by fam->word_ptrs (i.e. they should be independently malloc'd),
   because fam->word_ptrs can move during a realloc.
   As with fam->word_ptrs, the final pointer should be NULL.
*/
char **get_new_word_list(Family *fam) {
    int size = fam->num_words;
    char **new_list = malloc(sizeof(char*) * (size + 1));

    for (int i = 0; i < size; i++) {
        new_list[i] = &(*fam->word_ptrs[i]);
    }
    new_list[size] = NULL;

    return new_list;
}


/* Return a pointer to a random word from fam.
   Use rand (man 3 rand) to generate random integers.
*/
char *get_random_word_from_family(Family *fam) {
    int i = rand() % fam->num_words;
    return fam->word_ptrs[i];
}
