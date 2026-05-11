#define M1(x) x
#define M2(x) M1(x)
void test() {
    M2(int i = 0);
    M1(int j = 0);
}
