#ifndef MY_APP_UNIQPTR_H
#define MY_APP_UNIQPTR_H

template <typename T>
class UniqPtr
{
private:
    T* ptr;
public:
    explicit UniqPtr(T* ptr_a = nullptr) : ptr(ptr_a) {}
    UniqPtr(const UniqPtr& other) = delete;
    UniqPtr& operator=(const UniqPtr& other) = delete;
    UniqPtr(UniqPtr&& other) noexcept
    {
        other.ptr = nullptr;
    }
    UniqPtr& operator=(UniqPtr&& other) noexcept
    {
        if (this != &other)
        {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }

        return *this;
    }
    T& operator*() const {return *ptr;}
    T* operator->() const {return ptr;}
    T* Get() const noexcept {return ptr;}

    template <typename U>
    UniqPtr(const UniqPtr<U>&& other) noexcept : ptr(other.Release()) {}


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
    UniqPtr(const UniqPtr& other) = delete;

    UniqPtr& operator=(const UniqPtr& other) = delete;

    UniqPtr(UniqPtr&& other) noexcept : ptr(other.ptr), count(other.count)
    {
        other.ptr = nullptr;
        other.count = 0;
    }

    UniqPtr& operator=(UniqPtr&& other) noexcept
    {
        if (this != &other)
        {
            delete[] ptr;
            ptr = other.ptr;
            count = other.count;
            other.ptr = nullptr;
            other.count = 0;
        }

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

#endif //MY_APP_UNIQPTR_H