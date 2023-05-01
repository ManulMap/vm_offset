#include "vm_offs/offset.hh"

// clang-format off
struct Base {
    virtual auto base_f0() -> void {}
    virtual auto base_f1(int a, int b) -> int { return a + b; }
    virtual auto base_f2(int a, int b) -> int { return a * b; }
    virtual auto base_f3() -> void {}
};

struct Derived : Base {
    auto base_f2(int a, int b) -> int override {return a / b;}
    auto base_f3() -> void override {}
    virtual auto der_f4() -> void {}
    virtual auto der_f5(int a, int b) -> int { return a + b; }
    virtual auto der_f6() -> void {}
};

// clang-format on

int main()
{
    if (const auto base_f2_override_index = vm_offs::get_vm_index(&Derived::base_f2);
        base_f2_override_index != 2)
        return 1;

    if (const auto base_f3_override_index = vm_offs::get_vm_index(&Derived::base_f3);
        base_f3_override_index != 3)
        return 1;

    if (const auto der_f4_index = vm_offs::get_vm_index(&Derived::der_f4); der_f4_index != 4)
        return 1;

    if (const auto der_f5_index = vm_offs::get_vm_index(&Derived::der_f5); der_f5_index != 5)
        return 1;

    if (const auto der_f6_index = vm_offs::get_vm_index(&Derived::der_f6); der_f6_index != 6)
        return 1;

    return 0;
}