#include "allocator.hpp"

BumpAllocator::BumpAllocator(size_t size, void* memory)
    :
    m_base_address(reinterpret_cast<uintptr_t>(memory)),
    m_max_address(m_base_address + size),
    m_stack_ptr(0)
{
    //
}

void* BumpAllocator::allocate(size_t size, size_t alignment)
{
    size_t const alloc_size = BONSAI_ALIGN_ADDR(size, alignment);
    if (m_base_address + m_stack_ptr + alloc_size > m_max_address)
    {
        return nullptr;
    }

    uintptr_t const new_allocation = BONSAI_ALIGN_ADDR(m_base_address + m_stack_ptr, alignment);
    m_stack_ptr += alloc_size;
    return reinterpret_cast<void*>(new_allocation);
}

void BumpAllocator::free([[maybe_unused]] void* ptr)
{
    //
}

[[nodiscard]] uintptr_t BumpAllocator::get_marker() const
{
    return m_base_address + m_stack_ptr;
}

void BumpAllocator::reset(uintptr_t marker)
{
    if (marker < m_base_address
        || marker > m_max_address
        || marker > m_stack_ptr)
    {
        return;
    }

    m_stack_ptr = marker;
}
