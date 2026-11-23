#ifndef SMARTPTRS_H
#define SMARTPTRS_H

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
class UniqPtr
{
private:
    T* ptr;
public:
    explicit UniqPtr(T* ptr_a = nullptr) : ptr(ptr_a) {}
    UniqPtr(const UniqPtr& other)
    {
        if (other.ptr != nullptr)
        {
            ptr = new T(*other.ptr);
        }
        else ptr = nullptr;
    }
    UniqPtr& operator=(const UniqPtr& other)
    {
        if (this == &other) return *this;

        delete ptr;

        if (other.ptr != nullptr)
        {
            ptr = new T(*other.ptr);
        }
        else ptr = nullptr;

        return *this;
    }
    UniqPtr(UniqPtr&& other) noexcept
    {
        ptr = other.ptr;
        other.ptr = nullptr;
    }
    UniqPtr& operator=(UniqPtr&& other) noexcept
    {
        if (this == &other) return *this;

        delete ptr;

        ptr = other.ptr;
        other.ptr = nullptr;

        return *this;
    }
    T& operator*() const {return *ptr;}
    T* operator->() const {return ptr;}
    T* Get() const noexcept {return ptr;}

    template <typename U>
    UniqPtr(const UniqPtr<U>& other)
    {
        if (other.Get() != nullptr)
        {
            ptr = new T(*other.Get());
        }
        else ptr = nullptr;
    }

    T* Release() noexcept
    {
        T* temp = ptr;
        ptr = nullptr;

        return temp;
    }

    void Reset(T* ptr_p = nullptr)
    {
        if (ptr != ptr_p)
        {
            delete ptr;
            ptr = ptr_p;
        }
    }

    explicit operator bool() const noexcept
    {
        return ptr != nullptr;
    }

    ~UniqPtr() { delete ptr;}
};

template <typename T>
class UniqPtr<T[]>
{
private:
    T* ptr;
    int count;
public:
    explicit UniqPtr(T* ptr_a = nullptr, int size = 0) : ptr(ptr_a), count(ptr_a ? size : 0) {}
    UniqPtr(const UniqPtr& other) : count(other.count)
    {
        if(other.ptr != nullptr && count > 0)
        {
            ptr = new T[count];
            for(int i = 0; i < count; i++) ptr[i] = other.ptr[i];
        }
        else
        {
            ptr = nullptr;
            count = 0;
        }
    }

    UniqPtr& operator=(const UniqPtr& other)
    {
        if(this == &other) return *this;

        delete[] ptr;

        count = other.count;
        if(other.ptr != nullptr && count > 0)
        {
            ptr = new T[count];
            for(int i = 0; i < count; i++) ptr[i] = other.ptr[i];
        }
        else
        {
            ptr = nullptr;
            count = 0;
        }

        return *this;
    }

    UniqPtr(UniqPtr&& other) noexcept : ptr(other.ptr), count(other.count)
    {
        other.ptr = nullptr;
        other.count = 0;
    }

    UniqPtr& operator=(UniqPtr&& other) noexcept
    {
        if (this == &other) return *this;

        delete[] ptr;

        ptr = other.ptr;
        count = other.count;

        other.ptr = nullptr;
        other.count = 0;

        return *this;
    }

    T& operator[](int index) const
    {
        return ptr[index];
    }

    T* Get() const noexcept { return ptr; }
    int Size() const noexcept { return count; }

    T* Release() noexcept
    {
        T* temp = ptr;
        ptr = nullptr;
        count = 0;
        return temp;
    }

    void Reset(T* ptr_p = nullptr, int new_size = 0) noexcept
    {
        if (ptr != ptr_p)
        {
            delete[] ptr;
            ptr = ptr_p;
            count = ptr_p ? new_size : 0;
        }
    }

    explicit operator bool() const noexcept
    {
        return ptr != nullptr;
    }

    ~UniqPtr()
    {
        delete[] ptr;
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

#endif //SMARTPTRS_H`