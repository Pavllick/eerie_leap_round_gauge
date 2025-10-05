#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_SHARED_MULTI_HEAP
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
        void* pointer = nullptr;

    #ifdef CONFIG_SHARED_MULTI_HEAP
        pointer = shared_multi_heap_aligned_alloc(SMH_REG_ATTR_EXTERNAL, kAlignment, n * sizeof(T));
    #else
        pointer = k_malloc(n * sizeof(T));
    #endif

        if(pointer == nullptr) {
            LOG_MODULE_DECLARE(heap_allocator_logger);
            LOG_ERR("Failed to allocate %zu bytes for %zu elements of type %s\n", n * sizeof(T), n, typeid(T).name());
            throw std::bad_alloc();
        }

        return static_cast<T*>(pointer);
    }

    void deallocate(T* p, size_t) noexcept {
    #ifdef CONFIG_SHARED_MULTI_HEAP
        shared_multi_heap_free(p);
    #else
        k_free(p);
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
    alloc_deleter() = default;
    alloc_deleter(const HeapAllocator<T>& obj) : obj_(obj) { }

    using pointer = T*;

    void operator()(pointer p) const {
        if (p) {
            HeapAllocator<T> allocator(obj_);
            std::allocator_traits<HeapAllocator<T>>::destroy(allocator, p);
            std::allocator_traits<HeapAllocator<T>>::deallocate(allocator, p, 1);
        }
    }

private:
    HeapAllocator<T> obj_;
};

template<typename T, typename... Args>
auto allocate_unique(const HeapAllocator<T>& obj, Args&&... args) {
    using AT = std::allocator_traits<HeapAllocator<T>>;
    static_assert(std::is_same<typename AT::value_type, std::remove_cv_t<T>>::value,
                "Allocator has the wrong value_type");

    HeapAllocator<T> allocator(obj);
    auto p = allocator.allocate(1);

    try {
        AT::construct(allocator, p, std::forward<Args>(args)...);
        using D = alloc_deleter<T>;
        return std::unique_ptr<T, D>(p, D(allocator));
    } catch (...) {
        allocator.deallocate(p, 1);
        throw;
    }
}

template<typename T>
using ext_unique_ptr = std::unique_ptr<T, alloc_deleter<T>>;

template <typename T, typename... Args>
ext_unique_ptr<T> make_unique_ext(Args&&... args) {
    return allocate_unique<T>(HeapAllocator<T>(), std::forward<Args>(args)...);
}

using ExtVector = std::vector<uint8_t, HeapAllocator<uint8_t>>;

} // namespace eerie_leap::utilities::memory
