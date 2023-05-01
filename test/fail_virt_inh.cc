#include "vm_offs/offset.hh"

// clang-format off
struct VirtualBase {
    virtual void virtual_base_f0() {}

    virtual void virtual_base_f1() {}
};

struct VirtualDerived : virtual VirtualBase {
    virtual void virtual_der_f0() {}

    virtual void virtual_der_f1() {}
};
// clang-format on

int main()
{
    const auto compilation_error = vm_offs::get_vm_index(&VirtualDerived::virtual_der_f1);

    return 0;
}