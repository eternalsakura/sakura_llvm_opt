#include<stdio.h>

int main(int argc, char * argv[]) {
    int x = argc+5;
    int y = argc;
    if (argc > 4) {
        y++;
        x = x+5;
        y++;
    }
    else {
        y++;
        x = argc-2;
        y++;
    }
    y = 57;
    printf("%d", x);
    return 0;
}