#include "vm_offs/offset.hh"

// clang-format off
struct A {
    virtual void f0(){}
    virtual void f1() {}
    virtual int f2(int x, int y){ return x + y; }
    virtual float f3(float x){ return x * 2.0f; }
    //  virtual void f4(int x, float y) const {} <----------*
    virtual void f4(int x, float y){} //                    | 
    virtual void pad(){} //                                 | microsoft ABI const vm reordering
    virtual void pad2(){} //                                | 
    virtual void pad3(){} //                                | 
    virtual void f4(int x, float y) const{} // -------------*
};

// clang-format on

int main()
{
    const auto f0_index = vm_offs::get_vm_index(&A::f0);
    if (f0_index != 0)
        return 1;

    const auto f2_index = vm_offs::get_vm_index(&A::f2);
    if (f2_index != 2)
        return 2;

    const auto f3_index = vm_offs::get_vm_index(&A::f3);
    if (f3_index != 3)
        return 3;

    using F4Mut = void (A::*)(int, float);
    F4Mut f4_m  = &A::f4;

    const auto f4_mut_index = vm_offs::get_vm_index(f4_m);

    using F4Const = void (A::*)(int, float) const;
    F4Const f4_c  = &A::f4;

    const auto f4_const_index = vm_offs::get_vm_index(f4_c);

    constexpr auto abi = vm_offs::get_abi();
    if constexpr (abi == vm_offs::ABI::ClangWin || abi == vm_offs::ABI::Msvc) {
        if (f4_mut_index != 5)
            return 5;

        if (f4_const_index != 4)
            return 4;
    }
    else {
        if (f4_mut_index != 4)
            return 6;

        if (f4_const_index != 8)
            return 7;
    }

    return 0;
}