// Provide minimal operator delete implementations for AVR toolchain that
// may not include sized-deallocation symbols. These are no-op because
// this firmware doesn't use dynamic deallocation at runtime.

#include <stddef.h>

void operator delete(void* ptr) noexcept {
    (void)ptr;
}

void operator delete(void* ptr, size_t) noexcept {
    (void)ptr;
}
