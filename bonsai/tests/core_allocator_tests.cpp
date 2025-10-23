#include <gtest/gtest.h>
#include <core/allocator.hpp>

TEST(core_allocator, bump_alloctor_construct_class)
{
    class Foo {};
    size_t const memory_size = 1 * sizeof(Foo);
    void* memory = std::malloc(memory_size);

    BumpAllocator alloc(memory_size, memory);
    Foo* foo = alloc.alloc_object<Foo>();
    EXPECT_EQ(foo, memory);

    Foo* foo_null = alloc.alloc_object<Foo>();
    EXPECT_EQ(foo_null, nullptr);

    alloc.destroy(foo);
    std::free(memory);
}

TEST(core_allocator, bump_allocator_allocate)
{
    BumpAllocator alloc(2048);
    void* addr = alloc.allocate(1024, 16);
    EXPECT_EQ(BONSAI_ALIGN_ADDR(reinterpret_cast<uintptr_t>(addr), 16), reinterpret_cast<uintptr_t>(addr));
}

TEST(core_allocator, bump_allocator_allocate_alignment_larger_than_allocation)
{
    BumpAllocator alloc(2048);
    void* addr = alloc.allocate(64, 256);
    EXPECT_EQ(BONSAI_ALIGN_ADDR(reinterpret_cast<uintptr_t>(addr), 256), reinterpret_cast<uintptr_t>(addr));
}

TEST(core_allocator, bump_allocator_reset)
{
    BumpAllocator alloc(2048);
    void* addr1 = alloc.allocate(512, 256);
    EXPECT_EQ(BONSAI_ALIGN_ADDR(reinterpret_cast<uintptr_t>(addr1), 256), reinterpret_cast<uintptr_t>(addr1));
    uintptr_t const marker = alloc.get_marker();

    void* addr2 = alloc.allocate(512, 256);
    EXPECT_EQ(BONSAI_ALIGN_ADDR(reinterpret_cast<uintptr_t>(addr2), 256), reinterpret_cast<uintptr_t>(addr2));
    alloc.reset(marker);

    void* addr3 = alloc.allocate(1024, 256);
    EXPECT_EQ(BONSAI_ALIGN_ADDR(reinterpret_cast<uintptr_t>(addr3), 256), reinterpret_cast<uintptr_t>(addr3));
    EXPECT_EQ(addr2, addr3);
}
