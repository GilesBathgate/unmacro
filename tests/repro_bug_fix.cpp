#define CGAL_forall_shalfedges(eee, D) for(int eee = 0; eee < 10; ++eee)
#define CGAL_NEF_TRACEN(x)

typedef int SHalfedge_handle;

void test() {
    SHalfedge_handle eee;CGAL_forall_shalfedges(eee, D)CGAL_NEF_TRACEN("|" << eee);CGAL_NEF_TRACEN(" ");
}
