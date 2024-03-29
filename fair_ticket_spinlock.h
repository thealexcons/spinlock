#ifndef SPIN_LOCK_TICKET_OPTIMISED_H
#define SPIN_LOCK_TICKET_OPTIMISED_H

#include <emmintrin.h>

#include <atomic>
#include <string>

/*
Ticket spinlocks are used in practice in NUMA systems. Although it has a higher overhead than standard
spinlocks, it scales better for NUMA systems.
Why? Processes on the same NUMA node as the standard spinlock have an unfair advantage in obtaining the lock.
Processes on remote NUMA nodes experience lock starvation and degraded performance.
*/

class fair_ticket_spinner {
 public:
  fair_ticket_spinner() : next_ticket_(0), now_serving_(0) {}

  void lock() {
    const auto ticket = next_ticket_.fetch_add(1);
    while (true) {
      size_t ns = now_serving_.load();
      if (ns == ticket) {
        break;
      }
      // Back off for a number of iterations proportional to the "distance"
      // between my ticket and the currently-served ticket.
      for (size_t i = 0; i < ticket - ns; i++) {
        _mm_pause();
      }
    }
  }

  void unlock() { now_serving_.store(now_serving_.load() + 1); }

 private:
  std::atomic<size_t> next_ticket_;
  std::atomic<size_t> now_serving_;
};

#endif  // SPIN_LOCK_TICKET_OPTIMISED_H
