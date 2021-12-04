#pragma once

#include <atomic>
#include <emmintrin.h>

class active_spinner {
 public:
  active_spinner() : lock_bit_(false) {}
  active_spinner(const active_spinner&) = delete;
  active_spinner& operator=(const active_spinner&) = delete;

  void lock() {

    // The load part of the exchange() should synchronise with the store()
    // in the Unlock() method.
    // Therefore, we can use a release-store and acquire-load synchronisation
    // such that the "load/read" part of the exchange() RMW is acquire and the
    // "store/write" part of the RMW is relaxed.
    while (lock_bit_.exchange(true, std::memory_order_acquire)) {
      do {
        for (volatile size_t i = 0; i < 100; i++) {
        }
        // The locally spinning load can be relaxed, since it does not have
        // to synchronise with other operations, it is just used a heuristic
        // to avoid repeated RMWs very quickly.
      } while (lock_bit_.load(std::memory_order_relaxed));
    }
  }

  void unlock() {
    // Synchronise with the load of the exchange() by releasing/committing
    // our memory to the atomic location.
    lock_bit_.store(false, std::memory_order_release);
  }

 private:
  std::atomic<bool> lock_bit_;
};
