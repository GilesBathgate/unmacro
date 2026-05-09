#include <vector>
#include <string>

#define UNUSED_MACRO(x)

template<typename T>
void process(T val) {
    UNUSED_MACRO(val);
}

int main() {
    std::vector<std::string> v;
    process(v);
    return 0;
}
