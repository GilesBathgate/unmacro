#include <stdio.h>

#ifdef USE_LOGGING
#define LOG(x) printf("%s\n", x)
#else
#define LOG(x)
#endif

int main() {
    LOG("hello");
    if (1) LOG("world");
    return 0;
}
