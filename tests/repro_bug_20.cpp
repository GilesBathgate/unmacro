typedef int SHalfedge_const_handle;
#define CGAL_NEF_TRACEN(x)
#define CGAL_forall_shalfedges(ceee,E) for(ceee=0; ceee<10; ++ceee)

void test() {
SHalfedge_const_handle ceee;
    CGAL_NEF_TRACEN("---------------------");
    CGAL_forall_shalfedges(ceee,E)
      CGAL_NEF_TRACEN("|" << ceee);
    CGAL_NEF_TRACEN(" ");
}
