#include<stdio.h>

int main(int argc, char * argv[]) {
    int x = argc+5;
    int y = argc-2;
    int z = x+y;
    int g = z-4;
    for (int i=0; i<z-3; i++) {
        int a = i+9;
        int b = a+z;
        if (b>g-3) {
            int e = a*b;
        }
        else {
            int o = a+b;
            break;
        }
        int t = z-4;
        z = b-i;
    }
    int s = x+z;
    int f = g+s;
    printf("%d", f);
}