#include<stdio.h>

int main(int argc, char * argv[]) {
    int x = argc+5;
    int y = x+9;
    int z = x+y;
    for (int i=0; i<z; i++) {
        int a = i+9;
        int b = a+z;
        z = b-i;
        printf("%d", z);
    }
    int t = z-4;
    int s = x+z;
    int f = t+s;
    printf("%d", f);
}
