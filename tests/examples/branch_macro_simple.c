#include <stdio.h>

#ifdef USE_TRACE
#define MY_TRACE(t) if(1) \
    { fprintf(stderr, "%s\n", t); }
#else
#define MY_TRACE(t) ((void)0)
#endif

int main() {
    MY_TRACE("test1");

    if (1) {
        MY_TRACE("test2");
    }

    MY_TRACE("test3");

    return 0;
}
