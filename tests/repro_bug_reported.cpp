#define CGAL_forall_shalfedges_of(x,V) for(x = (V)->shalfedges_begin(); x != (V)->shalfedges_end(); ++x)
#define CGAL_NEF_TRACEN(x)

typedef int* SHalfedge_handle;
struct Face { int* shalfedges_begin(); int* shalfedges_end(); };

void test(Face* E) {
    SHalfedge_handle eee;
    CGAL_forall_shalfedges_of(eee, E)
      CGAL_NEF_TRACEN("test");
}
