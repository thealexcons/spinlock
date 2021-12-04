
// An RAII wrapper for spinlocks
template <class spinner>
class spinlock {
private:
    spinner _spinner{};

public:
    spinlock() {
        _spinner.lock();
    }

    ~spinlock() {
        _spinner.unlock();
    }
};
