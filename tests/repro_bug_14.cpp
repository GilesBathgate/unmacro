#define FOR_LOOP(eee) for(int eee=0; eee<10; ++eee)
#define TRACE(x)

void test() {
    int SHalfedge_handle_eee;
    FOR_LOOP(eee)
        TRACE(eee);
    TRACE(2);
}
