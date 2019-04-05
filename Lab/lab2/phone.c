#include <stdio.h>

int main() {
    char phone[11];
    int num;
    int result = 0;

    scanf("%s", phone);
    scanf("%d", &num);

    if (num == -1) {
        printf("%s\n", phone);
    } else if ((num >= 0) & (num <= 9)) {
        printf("%c\n", phone[num]);
    } else {
        printf("ERROR\n");
        result = 1;
    }

    return result;
}
