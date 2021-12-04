#pragma once

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <atomic>
#include <string>

class adaptive_futex_lock {
 public:
  adaptive_futex_lock() : state_(kFree) {}

  void lock() {
    int old = 0;
    for (size_t i = 0; i < 40; i++) {
      old = cmpxg(kFree, kLockedNoWaiters);
      if (kFree == old) {
        // We got the lock!
        return;
      }
    }
    do {
      if (old == kLockedWaiters ||
          cmpxg(kLockedNoWaiters, kLockedWaiters) != kFree) {
        syscall(SYS_futex, reinterpret_cast<int*>(&state_), FUTEX_WAIT,
                kLockedWaiters, nullptr, nullptr, 0);
      }
      old = cmpxg(kFree, kLockedWaiters);
    } while (kFree != old);
    // We have the lock!
  }

  void unlock() {
    if (state_.fetch_sub(1) == kLockedWaiters) {
      state_.store(kFree);
      syscall(SYS_futex, reinterpret_cast<int*>(&state_), FUTEX_WAKE, 1,
              nullptr, nullptr, 0);
    }
    // No store to state needed if fetch_sub(1) returned kLockedNoWaiters,
    // because that means it is now kFree.
  }

 private:
  // Perform compare-exchange(expected, desired) on state_
  // Return state_'s old value
  int cmpxg(int expected, int desired) {
    state_.compare_exchange_strong(expected, desired);
    return expected;
  }

  const int kFree = 0;
  const int kLockedNoWaiters = 1;
  const int kLockedWaiters = 2;

  std::atomic<int> state_;
};
