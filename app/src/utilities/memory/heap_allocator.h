#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#ifdef CONFIG_SHARED_MULTI_HEAP
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <soc/soc_memory_layout.h>
#include <zephyr/multi_heap/shared_multi_heap.h>
#endif

// #ifdef CONFIG_SHARED_MULTI_HEAP

// void* operator new(std::size_t sz);
// void operator delete(void* ptr) noexcept;

// #endif // CONFIG_SHARED_MULTI_HEAP

namespace eerie_leap::utilities::memory {

template <typename T>
class HeapAllocator {
private:
    static constexpr size_t kAlignment = alignof(T) > 32 ? alignof(T) : 32;

public:
    using value_type = T;

    HeapAllocator() = default;

    template <typename U>
    constexpr HeapAllocator(const HeapAllocator<U>& obj) noexcept {}

    T* allocate(size_t n) {
    #ifdef CONFIG_SHARED_MULTI_HEAP
        return static_cast<T*>(shared_multi_heap_aligned_alloc(SMH_REG_ATTR_EXTERNAL, kAlignment, n * sizeof(T)));
    #else
        return static_cast<T*>(::operator new(n * sizeof(T)));
    #endif
    }

    void deallocate(T* p, size_t) noexcept {
    #ifdef CONFIG_SHARED_MULTI_HEAP
        shared_multi_heap_free(p);
    #else
        ::operator delete(p);
    #endif
    }
};

template <class T, class U>
bool operator==(const HeapAllocator<T>& lhs, const HeapAllocator<U>& rhs) { return true; }

template <class T, class U>
bool operator!=(const HeapAllocator<T>& lhs, const HeapAllocator<U>& rhs) { return false; }

template <typename T, typename... Args>
std::shared_ptr<T> make_shared_ext(Args&&... args) {
    return std::allocate_shared<T>(HeapAllocator<T>(), std::forward<Args>(args)...);
}

template<typename T>
struct alloc_deleter {
    alloc_deleter(const T& obj) : obj_(obj) { }

    using pointer = std::allocator_traits<T>::pointer;

    void operator()(pointer p) const {
        T object(obj_);
        std::allocator_traits<T>::destroy(object, std::addressof(*p));
        std::allocator_traits<T>::deallocate(object, p, 1);
    }

private:
    T obj_;
};

template<typename T, typename... Args>
auto allocate_unique(const HeapAllocator<T>& obj, Args&&... args) {
    using AT = std::allocator_traits<HeapAllocator<T>>;
    static_assert(std::is_same<typename AT::value_type, std::remove_cv_t<T>>{}(),
                "Allocator has the wrong value_type");

    HeapAllocator<T> allocator(obj);
    auto p = allocator.allocate(1);

    try {
        ::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
        using D = alloc_deleter<HeapAllocator<T>>;
        return std::unique_ptr<T, D>(p, D(allocator));
    } catch (...) {
        allocator.deallocate(p, 1);
        throw;
    }
}

template <typename T, typename... Args>
std::unique_ptr<T, alloc_deleter<HeapAllocator<T>>> make_unique_ext(Args&&... args) {
    return allocate_unique<T>(HeapAllocator<T>(), std::forward<Args>(args)...);
}

using ExtVector = std::vector<uint8_t, HeapAllocator<uint8_t>>;

} // namespace eerie_leap::utilities::memory
