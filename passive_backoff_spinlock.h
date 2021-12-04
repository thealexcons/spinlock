#pragma once

#include <atomic>
#include <algorithm>
#include <emmintrin.h>

namespace {
    constexpr int min_backoff = 1 << 2;
    constexpr int max_backoff = 1 << 10;
}

// Spinlock implementation for x86 using exponential backoff
class passive_spinner {
private:
    std::atomic<bool> _locked;

public:
    passive_spinner() : _locked{false} {}
    passive_spinner(const passive_spinner&) = delete;
    passive_spinner& operator=(const passive_spinner&) = delete;

    void lock() {
        int backoff_iters = min_backoff;

        while (true) {
            // Return immediately if locked was already false
            if (!_locked.exchange(true)) {
                return;
            }

            // Wait on a read-only copy which gets cached locally to avoid
            // cache invalidation among other waiting threads.
            // Use exponential backoff (to reduce reads)
            do {
                for (int i = 0; i < backoff_iters; ++i) {
                    _mm_pause();    // Intel x86 intrinsic to add a delay
                }

                backoff_iters = std::min(backoff_iters << 1, max_backoff);
            } while (_locked.load());
        }
    }

    void unlock() {
        _locked.store(false);
    }

};
