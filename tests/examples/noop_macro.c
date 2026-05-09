#include <stdio.h>

#ifdef USE_LOGGING
#define LOGRESULT(x) printf("%d\n", x)
#else
#define LOGRESULT(x) (void)0
#endif

int get_result() { return 42; }

int main() {
    LOGRESULT(get_result());
    LOGRESULT(get_result(););
    return 0;
}
