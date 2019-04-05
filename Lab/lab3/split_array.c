#include <stdio.h>
#include <stdlib.h>

/* Return a pointer to an array of two dynamically allocated arrays of ints.
   The first array contains the elements of the input array s that are
   at even indices.  The second array contains the elements of the input
   array s that are at odd indices.

   Do not allocate any more memory than necessary.
*/
int **split_array(const int *s, int length) {
    int **spt_arr = malloc(sizeof(int*) * 2);
    int quo = length / 2;
    int rem = length % 2;
    // Question: why calculating in malloc directlly cause allocating more bits?(not bytes)
    spt_arr[0] = malloc(sizeof(int) * (quo + rem));
    spt_arr[1] = malloc(sizeof(int) * quo);
    int i;
    for (i = 0; i < quo; i++) {
        spt_arr[0][i] = s[2 * i];
        spt_arr[1][i] = s[2 * i + 1];
    }
    if (rem != 0) {
        spt_arr[0][i] = s[2 * i];
    }
    return spt_arr;
}

/* Return a pointer to an array of ints with size elements.
   - strs is an array of strings where each element is the string
     representation of an integer.
   - size is the size of the array
 */

int *build_array(char **strs, int size) {
    int *full_arr = malloc(sizeof(int) * size);
    for(int i = 0; i < size; i++) {
        full_arr[i] = strtol(strs[i + 1], NULL, 10);
    }
    return full_arr;
}


int main(int argc, char **argv) {
    /* Replace the comments in the next two lines with the appropriate
       arguments.  Do not add any additional lines of code to the main
       function or make other changes.
     */
    int *full_array = build_array(argv, argc - 1);
    int **result = split_array(full_array, argc - 1);

    printf("Original array:\n");
    for (int i = 0; i < argc - 1; i++) {
        printf("%d ", full_array[i]);
    }
    printf("\n");

    printf("result[0]:\n");
    for (int i = 0; i < argc/2; i++) {
        printf("%d ", result[0][i]);
    }
    printf("\n");

    printf("result[1]:\n");
    for (int i = 0; i < (argc - 1)/2; i++) {
        printf("%d ", result[1][i]);
    }
    printf("\n");
    free(full_array);
    free(result[0]);
    free(result[1]);
    free(result);
    return 0;
}
