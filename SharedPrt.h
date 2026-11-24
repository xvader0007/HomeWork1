#ifndef MY_APP_SHAREDPRT_H
#define MY_APP_SHAREDPRT_H

#include <atomic>

struct ControlBlock
{
    std::atomic<int> strong_count;

    explicit ControlBlock(int initial = 1) noexcept : strong_count(initial) {}

    void Add_ref() noexcept
    {
        strong_count.fetch_add(1, std::memory_order_relaxed); //второе для ускорение и безопасности
    }

    bool Release_ref() noexcept
    {
        return strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1; //второе завершает процессы
    }

    int Use_count() const noexcept
    {
        return strong_count.load(std::memory_order_relaxed); //атомарно читает
    }
};

template <typename T>
class ShrdPtr
{
private:
    T* ptr;
    ControlBlock* cb;

    template <typename U> friend class ShrdPtr;

    void Release() noexcept
    {
        if(cb != nullptr)
        {
            if(cb->Release_ref())
            {
                delete ptr;
                delete cb;
            }

            ptr = nullptr;
            cb = nullptr;
        }
    }
public:
    template <typename U>
    ShrdPtr(const ShrdPtr<U>& other) noexcept : ptr(other.ptr), cb(other.cb)
    {
        if (cb != nullptr)
        {
            cb->Add_ref();
        }
    }
    explicit ShrdPtr(T* ptr_a = nullptr) : ptr(ptr_a)
    {
        if(ptr != nullptr) cb = new ControlBlock(1);
        else cb = nullptr;
    }

    ShrdPtr(const ShrdPtr& other) noexcept : ptr(other.ptr), cb(other.cb)
    {
        if(cb != nullptr) cb->Add_ref();
    }

    ShrdPtr& operator=(const ShrdPtr& other) noexcept
    {
        if(this != &other)
        {
            Release();

            ptr = other.ptr;
            cb = other.cb;
            if(cb != nullptr) cb->Add_ref();
        }

        return *this;
    }

    ShrdPtr(ShrdPtr&& other) noexcept : ptr(other.ptr), cb(other.cb)
    {
        other.ptr = nullptr;
        other.cb = nullptr;
    }

    ShrdPtr& operator=(ShrdPtr&& other) noexcept
    {
        if (this != &other)
        {
            Release();

            ptr = other.ptr;
            cb = other.cb;

            other.ptr = nullptr;
            other.cb = nullptr;
        }
        return *this;
    }

    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    T* get() const noexcept { return ptr; }
    int Use_count() const noexcept { return cb ? cb->Use_count() : 0; }

    void Reset(T* ptr_p = nullptr)
    {
        if (ptr != ptr_p)
        {
            Release();
            ptr = ptr_p;
            cb = (ptr != nullptr) ? new ControlBlock(1) : nullptr;
        }
    }

    explicit operator bool() const noexcept
    {
        return ptr != nullptr;
    }

    ~ShrdPtr()
    {
        Release();
    }
};

template <typename T>
class ShrdPtr<T[]>
{
private:
    T* ptr;
    ControlBlock* cb;

    void Release() noexcept
    {
        if (cb != nullptr)
        {
            if (cb->Release_ref())
            {
                delete[] ptr;
                delete cb;
            }
            ptr = nullptr;
            cb = nullptr;
        }
    }

public:
    explicit ShrdPtr(T* ptr_a = nullptr) : ptr(ptr_a)
    {
        cb = (ptr != nullptr) ? new ControlBlock(1) : nullptr;
    }

    ShrdPtr(const ShrdPtr& other) noexcept : ptr(other.ptr), cb(other.cb)
    {
        if (cb != nullptr)
        {
            cb->Add_ref();
        }
    }

    ShrdPtr& operator=(const ShrdPtr& other) noexcept
    {
        if (this != &other)
        {
            Release();
            ptr = other.ptr;
            cb = other.cb;
            if (cb != nullptr)
            {
                cb->Add_ref();
            }
        }
        return *this;
    }

    ShrdPtr(ShrdPtr&& other) noexcept : ptr(other.ptr), cb(other.cb)
    {
        other.ptr = nullptr;
        other.cb = nullptr;
    }

    ShrdPtr& operator=(ShrdPtr&& other) noexcept
    {
        if (this != &other)
        {
            Release();
            ptr = other.ptr;
            cb = other.cb;
            other.ptr = nullptr;
            other.cb = nullptr;
        }
        return *this;
    }

    T& operator[](int index) const { return ptr[index]; }
    T* get() const noexcept { return ptr; }
    int Use_count() const noexcept { return cb ? cb->Use_count() : 0; }

    void Reset(T* ptr_p = nullptr)
    {
        if (ptr != ptr_p)
        {
            Release();
            ptr = ptr_p;
            cb = (ptr != nullptr) ? new ControlBlock(1) : nullptr;
        }
    }

    explicit operator bool() const noexcept { return ptr != nullptr; }

    ~ShrdPtr() { Release(); }
};

#endif //MY_APP_SHAREDPRT_H