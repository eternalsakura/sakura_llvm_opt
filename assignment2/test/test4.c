#include<stdio.h>

int main(int argc, char * argv[]) {
    int x = argc+5;
    int y = argc;
    int z = x+y;
    int b = x-y;
    if (argc > 4) {
        int z2 = x+y;
        int s = argc+1;
        int c = b+z;
        int d = s/3;
    }
    else {
        int s = argc+2;
        int e = b+z;
        int f = s/3;
    }
    int z_end = x*y;
    printf("%d", z);
    return 0;
}