#ifndef SMARTPTRS_H
#define SMARTPTRS_H

template <typename T>
class UniqPtr
{
private:
    T* ptr;
public:
    //конструктор
    explicit UniqPtr(T* ptr_a = nullptr) : ptr(ptr_a) {}
    //конструктор копирования
    UniqPtr(const UniqPtr& other)
    {
        if (other.ptr != nullptr)
        {
            ptr = new T(*other.ptr);
        }
        else ptr = nullptr;
    }
    //оператор присваивания
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
    //конструктор перемещения
    UniqPtr(UniqPtr&& other) noexcept
    {
        ptr = other.ptr;
        other.ptr = nullptr;
    }
    //оператор перемещения
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
    T* Get() const {return ptr;}

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

    //деструктор
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
    int* ref_count;

    template <typename U> friend class ShrdPtr;

    void Release() noexcept
    {
        if(ref_count != nullptr)
        {
            (*ref_count)--;
            if(*ref_count == 0)
            {
                delete ptr;
                delete ref_count;
            }

            ptr = nullptr;
            ref_count = nullptr;
        }
    }
public:
    template <typename U>
    ShrdPtr(const ShrdPtr<U>& other) noexcept : ptr(other.ptr), ref_count(other.ref_count)
    {
        if (ref_count != nullptr)
        {
            (*ref_count)++;
        }
    }
    explicit ShrdPtr(T* ptr_a = nullptr) : ptr(ptr_a)
    {
        if(ptr != nullptr) ref_count = new int(1);
        else ref_count = nullptr;
    }

    ShrdPtr(const ShrdPtr& other) noexcept : ptr(other.ptr), ref_count(other.ref_count)
    {
        if(ref_count != nullptr) (*ref_count)++;
    }

    ShrdPtr& operator=(const ShrdPtr& other) noexcept
    {
        if(this != &other)
        {
            Release();

            ptr = other.ptr;
            ref_count = other.ref_count;
            if(ref_count != nullptr) (*ref_count)++;
        }

        return *this;
    }

    ShrdPtr(ShrdPtr&& other) noexcept : ptr(other.ptr), ref_count(other.ref_count)
    {
        other.ptr = nullptr;
        other.ref_count = nullptr;
    }

    ShrdPtr& operator=(ShrdPtr&& other) noexcept
    {
        if (this != &other)
        {
            Release();

            ptr = other.ptr;
            ref_count = other.ref_count;

            other.ptr = nullptr;
            other.ref_count = nullptr;
        }
        return *this;
    }

    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    T* get() const noexcept { return ptr; }
    int use_count() const noexcept { return ref_count ? *ref_count : 0; }

    void reset(T* ptr_p = nullptr)
    {
        if (ptr != ptr_p)
        {
            Release();
            ptr = ptr_p;
            ref_count = (ptr != nullptr) ? new int(1) : nullptr;
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
    int* ref_count;

    void Release() noexcept
    {
        if (ref_count != nullptr)
        {
            --(*ref_count);
            if (*ref_count == 0)
            {
                delete[] ptr; // Вызываем delete[] для массива[cite: 4]
                delete ref_count;
            }
            ptr = nullptr;
            ref_count = nullptr;
        }
    }

public:
    explicit ShrdPtr(T* ptr_a = nullptr) : ptr(ptr_a)
    {
        ref_count = (ptr != nullptr) ? new int(1) : nullptr;
    }

    ShrdPtr(const ShrdPtr& other) noexcept : ptr(other.ptr), ref_count(other.ref_count)
    {
        if (ref_count != nullptr)
        {
            ++(*ref_count);
        }
    }

    ShrdPtr& operator=(const ShrdPtr& other) noexcept
    {
        if (this != &other)
        {
            Release();
            ptr = other.ptr;
            ref_count = other.ref_count;
            if (ref_count != nullptr)
            {
                ++(*ref_count);
            }
        }
        return *this;
    }

    ShrdPtr(ShrdPtr&& other) noexcept : ptr(other.ptr), ref_count(other.ref_count)
    {
        other.ptr = nullptr;
        other.ref_count = nullptr;
    }

    ShrdPtr& operator=(ShrdPtr&& other) noexcept
    {
        if (this != &other)
        {
            Release();
            ptr = other.ptr;
            ref_count = other.ref_count;
            other.ptr = nullptr;
            other.ref_count = nullptr;
        }
        return *this;
    }

    T& operator[](int index) const { return ptr[index]; }
    T* get() const noexcept { return ptr; }
    int use_count() const noexcept { return ref_count ? *ref_count : 0; }

    void reset(T* ptr_p = nullptr)
    {
        if (ptr != ptr_p)
        {
            Release();
            ptr = ptr_p;
            ref_count = (ptr != nullptr) ? new int(1) : nullptr;
        }
    }

    explicit operator bool() const noexcept { return ptr != nullptr; }

    ~ShrdPtr() { Release(); }
};

#endif //SMARTPTRS_H