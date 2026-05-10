typedef int* SHalfedge_handle;
#define CGAL_forall_shalfedges_of(x,V) for(x = (V)->shalfedges_begin(); x != (V)->shalfedges_end(); ++x)
#define CGAL_NEF_TRACEN(x)

struct Face {
    int* shalfedges_begin() { return (int*)0; }
    int* shalfedges_end() { return (int*)0; }
};

void test(Face* E) {
    SHalfedge_handle ceee; CGAL_forall_shalfedges_of(ceee, E) { CGAL_NEF_TRACEN("test"); }
}
