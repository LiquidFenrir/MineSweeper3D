#ifndef THREAD3DS_INC
#define THREAD3DS_INC

#include <3ds.h>
#include <utility>
#include <tuple>
#include <memory>
#include <mutex>
#include <chrono>

namespace ctr {

struct thread_base {
    void sleep_for(const std::chrono::nanoseconds& length)
    {
        svcSleepThread(length.count());
    }
};
struct cur_thread : public thread_base {
    Thread get_id()
    {
        return Thread(intptr_t(4));
    }
    Handle native_handle() const
    {
        return CUR_THREAD_HANDLE;
    }
};

class thread : public thread_base {
    Thread m_th{};

    template<class P_t>
    static void trampoline(void* v_arg)
    {
        auto const thread_arg = std::unique_ptr<P_t>(static_cast<P_t*>(v_arg));
        std::apply(thread_arg->second, thread_arg->first);
    }

public:
    struct meta {
        std::size_t stack_size;
        int prio;
        int core_id;
    };

    template<class F, typename... Args>
    thread(const meta& info, F&& func, Args&&... args)
    {
        using P_t = std::pair<std::tuple<Args...>, F&&>;
        auto arg = std::make_unique<P_t>(
            std::tuple<Args...>(args...),
            std::forward<F>(func)
        );
        m_th = threadCreate(&thread::trampoline<P_t>, arg.get(), info.stack_size, info.prio, info.core_id, false);
        if(m_th) arg.release();
    }

    thread() = default;
    thread(thread&&) = default;
    thread(const thread&) = delete;

    ~thread()
    {
        if(m_th)
        {
            svcBreak(USERBREAK_ASSERT);
        }
    }

    operator bool() const
    {
        return m_th != nullptr;
    }

    bool joinable() const noexcept
    {
        return m_th != threadGetCurrent();
    }

    Thread get_id()
    {
        return m_th;
    }
    Handle native_handle() const
    {
        return threadGetHandle(m_th);
    }
    void join(const u64 timeout = U64_MAX)
    {
        if(R_SUCCEEDED(threadJoin(m_th, timeout)))
        {
            threadFree(m_th);
            m_th = nullptr;
        }
    }
    void swap(thread& other) noexcept
    {
        std::swap(m_th, other.m_th);
    }
};

[[maybe_unused]] static inline cur_thread this_thread;

class mutex {
    LightLock m_lock;

public:
    mutex() noexcept
    {
        LightLock_Init(&m_lock);
    }
    mutex(const mutex&) = delete;

    void lock() noexcept
    {
        LightLock_Lock(&m_lock);
    }
    bool try_lock() noexcept
    {
        return LightLock_TryLock(&m_lock) == 0;
    }
    void unlock() noexcept
    {
        LightLock_Unlock(&m_lock);
    }

    LightLock* native_handle()
    {
        return &m_lock;
    }
};

class recursive_mutex {
    RecursiveLock m_lock;

public:
    recursive_mutex() noexcept
    {
        RecursiveLock_Init(&m_lock);
    }
    recursive_mutex(const recursive_mutex&) = delete;

    void lock() noexcept
    {
        RecursiveLock_Lock(&m_lock);
    }
    bool try_lock() noexcept
    {
        return RecursiveLock_TryLock(&m_lock) == 0;
    }
    void unlock() noexcept
    {
        RecursiveLock_Unlock(&m_lock);
    }

    RecursiveLock* native_handle()
    {
        return &m_lock;
    }
};

}

#endif
