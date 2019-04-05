#ifndef READING_H
#define READING_H

/* Maximum length of word to read. */
#define MAX_WORD_LENGTH 40

/* Maximum number of words in the dictionary. */
#define MAX_WORDS 130000

/* Dictionary file name */
#define DICTIONARY "dictionary.txt"

char **read_words(char *filename);
void deallocate_words(char **words);
char **prune_word_list(char **words, int len, int *words_remaining); // only for test!
char **get_word_list_of_length(char **words, int *len); // only for test!

#endif
