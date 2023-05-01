#include "vm_offs/offset.hh"

// clang-format off
struct BaseFirst {
    virtual void first_f0() {}
    virtual void first_f1() {}
};

struct BaseSecond {
    virtual void second_f0() {}
    virtual void second_f1() {}
};

struct MultiDerived : BaseFirst, BaseSecond {
    virtual void multi_der_f0() {}
    virtual void multi_der_f1() {}
};

// clang-format on

int main()
{
    auto compilation_error = vm_offs::get_vm_index(&MultiDerived::multi_der_f1);

    return 0;
}