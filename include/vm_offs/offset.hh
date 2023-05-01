#ifndef VM_OFFSET_HH
#define VM_OFFSET_HH

#include <cstddef>
#include <cstdint>
#include <bit>
#include <cassert>
#include <expected>

namespace vm_offs {

enum class ABI { Unk, Msvc, ClangWin, Itanium };

consteval auto get_abi() -> ABI
{
#if (defined(_MSC_VER) && defined(__clang__))
    return ABI::ClangWin;
#elif defined(_MSC_VER)
    return ABI::Msvc;
#elif (defined(__clang__) && !defined(_MSC_VER) || (defined(__GNUC__)))
    return ABI::ITANIUM;
#else
    return ABI::UNK;
#endif
}

constexpr auto k_abi = get_abi();

consteval auto is_debug() -> bool
{
#if defined(_DEBUG)
    return true;
#else
    return false;
#endif
}

consteval auto is_clang_optimizations() -> bool
{
    if (get_abi() == ABI::ClangWin) {
#if defined(__OPTIMIZE__)
        return true;
#else
        return false;
#endif
    }
    return false;
}

inline auto //
get_abs_address(const uintptr_t instruction_ptr, const std::uint32_t opcode_sz,
                const std::uint32_t instr_sz) -> std::uintptr_t
{
    const std::ptrdiff_t offset = *reinterpret_cast<int32_t*>(instruction_ptr + opcode_sz);
    return instruction_ptr + offset + instr_sz;
}

inline constexpr bool is64 = sizeof(void*) == 8;

template <class Ret, class T, typename... Args>
    requires std::is_polymorphic_v<T>
[[nodiscard]] auto //
get_vm_offset_msvc(Ret (T::*virtual_pmf)(Args...)) -> std::uint32_t
{
    static_assert(sizeof(virtual_pmf) == sizeof(std::uintptr_t),
                  "Multiple and virtual inheritance is not allowed");

    // ; x64-bit pmf thunk:
    // mov rax, qword ptr ds:[rcx]
    // jmp qword ptr ds:[rax+VM_OFFSET]

    std::uintptr_t thunk;

    if constexpr (is_debug()) {
        auto dbg_jmp = std::bit_cast<std::uintptr_t>(virtual_pmf);
        thunk        = get_abs_address(dbg_jmp, sizeof(std::byte), 5 * sizeof(std::byte));
    }
    else {
        thunk = std::bit_cast<std::uintptr_t>(virtual_pmf);
    }

    // ; 64-bit
    // mov rax, qword ptr ds:[rcx]; 48 8b 01
    // ; 32-bit
    // mov eax, dword ptr ds:[ecx]; 8b 00

    constexpr std::ptrdiff_t sz_mov = sizeof(std::byte) * (is64 ? 3 : 2);

    const std::uintptr_t jmp = thunk + sz_mov;

    enum JMP_RAX_OFFSET_TYPE : std::uint8_t {
        JMP_RAX_NO_OFFSET         = 0x20,
        JMP_RAX_OFFSET_ONE_BYTE   = 0x60,
        JMP_RAX_OFFSET_FOUR_BYTES = 0xA0
    };

    const auto jmp_offset_type =
        *reinterpret_cast<enum JMP_RAX_OFFSET_TYPE*>(jmp + sizeof(std::byte));

    assert((jmp_offset_type == JMP_RAX_NO_OFFSET || jmp_offset_type == JMP_RAX_OFFSET_ONE_BYTE ||
            jmp_offset_type == JMP_RAX_OFFSET_FOUR_BYTES) &&
           "Unsupported jmp opcode");

    if (jmp_offset_type == JMP_RAX_NO_OFFSET)
        return 0;

    if (jmp_offset_type == JMP_RAX_OFFSET_ONE_BYTE) {
        const auto offset = *reinterpret_cast<std::int8_t*>(jmp + 2 * sizeof(std::byte));

        assert(offset >= 0 && "Virtual function offset must be a positive or zero value");
        return offset;
    }
    const auto offset = *reinterpret_cast<std::int32_t*>(jmp + 2 * sizeof(std::byte));

    assert(offset >= 0 && "Virtual function offset must be a positive or zero value");
    return offset;
}

template <class Ret, class T, typename... Args>
    requires std::is_polymorphic_v<T>
[[nodiscard]] auto get_vm_offset_win_clang(Ret (T::*virtual_pmf)(Args...)) -> std::uint32_t
{
    static_assert(sizeof(virtual_pmf) == sizeof(std::uintptr_t),
                  "Multiple and virtual inheritance is not allowed");

    // ; x64-bit pmf thunk in release(-O1-O3) build:
    // mov rax, qword ptr ds:[rcx]; padding
    // mov rax, qword ptr ds:[rax + vm_offset]; <------ target mov
    // jmp rax
    //
    // ; x64-bit pmf thunk in debug(-O0) build:
    // mov qword ptr ss:[rsp], rcx; <--|
    // mov rcx, qword ptr ss:[rsp];    | padding (0xC bytes)
    // mov rax, qword ptr ds:[rcx]; <--|
    // mov rax, qword ptr ds:[rax+vm_offset] <------ target_mov
    // pop r10
    // jmp rax

    const auto thunk = std::bit_cast<std::uintptr_t>(virtual_pmf);

    constexpr std::ptrdiff_t sz_padding =
        sizeof(std::byte) * (is_debug() || !is_clang_optimizations() ? 0xC : (is64 ? 0x3 : 0x2));

    const std::uintptr_t target_mov = thunk + sz_padding;

    enum MovRaxOffsetType : std::uint8_t {
        K_MOV_RAX_NO_OFFSET         = 0x00,
        K_MOV_RAX_OFFSET_ONE_BYTE   = 0x40,
        K_MOV_RAX_OFFSET_FOUR_BYTES = 0x80
    };

    const auto mov_offset_type =
        *reinterpret_cast<enum MovRaxOffsetType*>(target_mov + sizeof(std::byte) * (is64 ? 2 : 1));

    assert((mov_offset_type == K_MOV_RAX_OFFSET_ONE_BYTE ||
            mov_offset_type == K_MOV_RAX_OFFSET_FOUR_BYTES || mov_offset_type) &&
           "Unsupported mov opcode type");

    if (mov_offset_type == K_MOV_RAX_NO_OFFSET)
        return 0;

    if (mov_offset_type == K_MOV_RAX_OFFSET_ONE_BYTE) {
        const auto offset =
            *reinterpret_cast<std::int8_t*>(target_mov + sizeof(std::byte) * (is64 ? 3 : 2));

        assert((offset >= 0) && "Virtual function offset must be a positive or zero value");
        return offset;
    }

    const auto offset =
        *reinterpret_cast<std::int32_t*>(target_mov + sizeof(std::byte) * (is64 ? 3 : 2));

    assert((offset >= 0) && "Virtual function offset must be a positive or zero value");
    return offset;
}

template <class Ret, class T, typename... Args>
    requires std::is_polymorphic_v<T>
[[nodiscard]] auto //
get_vm_offset_itanium(Ret (T::*virtual_pmf)(Args...)) -> std::uint32_t
{
    using Pmf = Ret (T::*)(Args...);

    struct ItaniumPvmf {
        std::ptrdiff_t vm_offset_plus1;
        std::ptrdiff_t this_adj;
    };

    union {
        Pmf pvmf;
        ItaniumPvmf it_pvmf;
    };

    pvmf = virtual_pmf;

    assert((it_pvmf.vm_offset_plus1 % sizeof(std::uintptr_t) != 1) &&
           "Argument must be a pointer to virtual function");

    assert((it_pvmf.this_adj == 0) && "Multiple and virtual inheritance is not allowed");

    const std::int32_t offset = it_pvmf.vm_offset_plus1 - 1;
    assert((offset >= 0) && "Virtual function offset must be a positive or zero value");

    return offset;
}

template <class Ret, class T, typename... Args>
    requires std::is_polymorphic_v<T>
[[nodiscard]] auto //
get_vm_index(Ret (T::*virtual_pmf)(Args...)) -> std::uint32_t
{
    static_assert(k_abi != ABI::Unk, "Unsupported ABI");

    if constexpr (k_abi == ABI::Msvc)
        return get_vm_offset_msvc(virtual_pmf) / sizeof(void*);

    if constexpr (k_abi == ABI::ClangWin)
        return get_vm_offset_win_clang(virtual_pmf) / sizeof(void*);

    return get_vm_offset_itanium(virtual_pmf) / sizeof(void*);
}

template <class Ret, class T, typename... Args>
    requires std::is_polymorphic_v<T>
[[nodiscard]] auto //
get_vm_index(Ret (T::*virtual_pmf)(Args...) const) -> std::uint32_t
{
    return get_vm_index(reinterpret_cast<Ret (T::*)(Args...)>(virtual_pmf));
}

} // namespace vm_offs

#endif // VM_OFFSET_HH