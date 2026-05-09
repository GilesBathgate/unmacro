#include <stdio.h>

#define MY_MACRO(x)

void test() {
    for(int i = 0; i < 5; ++i)
        MY_MACRO(i);
    printf("done\n");
}
