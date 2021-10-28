#ifndef UTILITIES_INC
#define UTILITIES_INC

#include <memory>
#include <cstdio>

namespace util {

struct file_deleter {
    void operator()(FILE* fh)
    {
        fclose(fh);
    }
};

template<class T, class D>
using ptr_for = std::unique_ptr<std::remove_pointer_t<T>, D>;

using file_ptr = ptr_for<FILE*, file_deleter>;

}

#endif
