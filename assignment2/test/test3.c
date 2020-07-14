#include <stdio.h>
int main(int argc, char * argv[]) {
    int x = argc+2;
    int y = argc+4;
    int z = argc+5;
    int s, t;
    switch (x) {
        case 3:
        {
            s = z+3;
            t = argc-4;
            break;
        }
        case 4:
        {
            t = x+4;
            s = t+x;
            break;
        }
        default:
        {
            int f = 25+argc;
            t = f+f;
            s = t+f;
        }
    }
    int g = 50+x;
    int h = y-7;
    int i = t+s;
    int j = z-3;
    printf("%d %d %d %d", g, h, i, j);
}