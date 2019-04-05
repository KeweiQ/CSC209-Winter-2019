#include <stdio.h>

int pt(char *phone, int num) {
    if (num == -1) {
        printf("%s\n", phone);
        return 0;
    } else if ((num >= 0) & (num <= 9)) {
        printf("%c\n", phone[num]);
        return 0;
    } else {
        printf("ERROR\n");
        return 1;
    }
}

int main(int argc, char **argv) {
    if (argc != 1) {
        printf("This program requires no command-line arguments!\n");
        return 1;
    }
    char phone[11];
    int num, result;
    printf("Please type in the phone number: ");
    scanf("%s", phone);
    printf("Please type in the indicator: ");
    scanf("%d", &num);
    result = pt(phone, num);
    return result;
}
