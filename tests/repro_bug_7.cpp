#define FOR_LOOP(eee) for(int eee=0; eee<10; ++eee)
#define TRACE(x)

void test() {
    int eee;
    FOR_LOOP(eee)
      TRACE(eee);
    TRACE(" ");
}
