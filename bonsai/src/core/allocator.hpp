#pragma once
#ifndef BONSAI_RENDERER_ALLOCATOR_HPP
#define BONSAI_RENDERER_ALLOCATOR_HPP

#include <cstdint>
#include <cstddef>
#include <utility>

#define BONSAI_ALIGN_ADDR(addr, alignment)  (((addr) + (alignment) - 1) & ~((alignment) - 1))

/// @brief Allocator interface, provides allocation behaviour for raw memory regions and RAII objects.
class IAllocator
{
public:
    virtual ~IAllocator() = default;

    /// @brief Allocate a chunk of memory.
    /// @param size Size of this allocation in bytes.
    /// @param alignment Byte alignment of this allocation.
    /// @return The start address of the allocated memory.
    virtual void* allocate(size_t size, size_t alignment) = 0;

    /// @brief Free an allocated chunk of memory.
    /// @param ptr Memory pointer to free.
    virtual void free(void* ptr) = 0;

    /// @brief Allocate an object in the memory space of the allocator.
    /// @tparam Type Object type to construct.
    /// @tparam Args Object constructor arguments
    /// @param args
    /// @return A pointer to the newly constructed object.
    template<typename Type, typename... Args>
    Type* alloc_object(Args&&... args)
    {
        void* ptr = allocate(sizeof(Type), alignof(Type));
        if (ptr == nullptr)
        {
            return nullptr;
        }

        Type* type = new (ptr) Type(std::forward<Args>(args)...);
        return type;
    }

    /// @brief Destroy and free an object allocated from this allocator.
    /// @param ptr Object pointer to destroy.
    template<typename Type>
    void destroy(Type* ptr)
    {
        if (ptr == nullptr)
        {
            return;
        }

        ptr->~Type();
        free((void*)ptr);
    }
};

/// @brief Bump allocator that linearly allocates memory blocks in a stack-like fashion.
class BumpAllocator : public IAllocator
{
public:
    /// @brief Create a new BumpAllocator
    /// @param size Managed memory block size in bytes.
    /// @param memory Optional, start address of an externally allocated memory region backing this allocator.
    explicit BumpAllocator(size_t size, void* memory = nullptr);
    ~BumpAllocator() = default;

    BumpAllocator(const BumpAllocator&) = delete;
    BumpAllocator& operator=(const BumpAllocator&) = delete;

    void* allocate(size_t size, size_t alignment) override;

    void free(void* ptr) override;

    /// @brief Get a stack marker for the allocator.
    /// @return A stack marker within the managed block.
    [[nodiscard]] uintptr_t get_marker() const;

    /// @brief Reset the bump allocator to a previously acquired marker.
    /// @param marker Stack marker to reset the allocator to.
    void reset(uintptr_t marker);

private:
    uintptr_t m_base_address    = 0;
    uintptr_t m_max_address     = 0;
    uintptr_t m_stack_ptr       = 0;
};

#endif //BONSAI_RENDERER_ALLOCATOR_HPP