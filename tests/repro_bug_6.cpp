#define MACRO1(x) for(int i=0; i<x; ++i)
#define MACRO2(x)

void test() {
    int x = 5;
    MACRO1(x)
        MACRO2(i);
    MACRO2(x);
}
