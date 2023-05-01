# vm offset
Convenient way to get virtual method offset from virtual table
using a pointer to member function(pmf)

This lib uses very tricky compiler implementation dependent methods to get the offset

## example
```c++
struct A {
    virtual void f0() {}
    virtual void f1() {}
    virtual void f2() {}
};

const auto f2_offset = vm_offs::get_vm_index(&A::f2);
```

Tested on msvc, clang for windows(clang, clang-cl), and godbolt(itanium abi clang and gcc).

If it doesn't work correctly for you, consider using these methods directly.
```c++
const auto f2_index_itanium = vm_offs::get_vm_offset_itanium(&A::f2);
const auto f2_index_msvc = vm_offs::get_vm_offset_itanium(&A::f2);
const auto f2_index_clang = vm_offs::get_vm_offset_win_clang(&A::f2);
```