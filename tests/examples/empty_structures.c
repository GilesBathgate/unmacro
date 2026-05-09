#define MY_MACRO(x)

void test() {
    if (1)
        MY_MACRO(1);

    for (int i = 0; i < 10; ++i) {
        MY_MACRO(i);
    }

    if (2) {
        MY_MACRO(2);
    }

    if (3) {
        MY_MACRO(3);
        int actual_stmt;
    }
}
