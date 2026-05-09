#include <iostream>

#ifdef USE_TRACE
#define MY_TRACE(t) if(true) \
    { std::cerr<< " "<<t<<std::endl; }
#else
#define MY_TRACE(t) (static_cast<void>(0))
#endif

int main() {
    MY_TRACE("test1");

    if (true)
        MY_TRACE("test2");
    else
        std::cout << "else" << std::endl;

    MY_TRACE("test3");

    return 0;
}
