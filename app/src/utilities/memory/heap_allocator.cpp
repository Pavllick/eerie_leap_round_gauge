#include "heap_allocator.h"

// #ifdef CONFIG_SHARED_MULTI_HEAP

// inline void* operator new(std::size_t sz) {
//     return shared_multi_heap_aligned_alloc(SMH_REG_ATTR_EXTERNAL, 32, sz);

// }

// inline void operator delete(void* ptr) noexcept {
//     shared_multi_heap_free(ptr);
// }

// #endif // CONFIG_SHARED_MULTI_HEAP
