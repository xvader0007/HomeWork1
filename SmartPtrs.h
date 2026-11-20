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

#endif //SMARTPTRS_H

/*
 *#pragma once

// ==========================================
// 1. UnqPtr: Ведущий указатель (Владелец)
// ==========================================
template <typename T>
class UnqPtr {
private:
    T* ptr;

public:
    // Конструктор
    explicit UnqPtr(T* p = nullptr) : ptr(p) {}

    // Конструктор копирования (Глубокое копирование, как требует ТЗ)
    UnqPtr(const UnqPtr& other) {
        ptr = (other.ptr != nullptr) ? new T(*other.ptr) : nullptr;
    }

    // Подтипизация для UnqPtr: копируем объект производного класса в базовый
    template <typename U>
    UnqPtr(const UnqPtr<U>& other) {
        ptr = (other.get() != nullptr) ? new T(*other.get()) : nullptr;
    }

    // Оператор присваивания
    UnqPtr& operator=(const UnqPtr& other) {
        if (this != &other) {
            delete ptr;
            ptr = (other.ptr != nullptr) ? new T(*other.ptr) : nullptr;
        }
        return *this;
    }

    // Деструктор
    ~UnqPtr() {
        delete ptr;
    }

    // Доступ
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    T* get() const { return ptr; }
};


// ==========================================
// 2. ShrdPtr: Дескриптор (Наблюдатель с подсчетом)
// ==========================================
template <typename T>
class ShrdPtr {
private:
    UnqPtr<T>* master;     // Указывает строго на UnqPtr
    size_t* ref_count;     // Динамический счетчик ссылок

    // Делаем все инстанции ShrdPtr дружественными для реализации подтипизации
    template <typename U> friend class ShrdPtr;

    // Вспомогательный метод очистки
    void release() {
        if (ref_count) {
            (*ref_count)--;
            if (*ref_count == 0) {
                delete master;     // Удаляем владельца (он сам удалит T*)
                delete ref_count;  // Удаляем счетчик
            }
        }
    }

public:
    // Конструктор. Принимает динамически выделенный UnqPtr!
    explicit ShrdPtr(UnqPtr<T>* m = nullptr) : master(m) {
        ref_count = (master != nullptr) ? new size_t(1) : nullptr;
    }

    // Конструктор копирования (просто увеличиваем счетчик)
    ShrdPtr(const ShrdPtr& other) : master(other.master), ref_count(other.ref_count) {
        if (ref_count) {
            (*ref_count)++;
        }
    }

    // Подтипизация (Subtyping): C1 ⊆ C2
    template <typename U>
    ShrdPtr(const ShrdPtr<U>& other) {
        // Защита: проверяем на этапе компиляции, можно ли привести U* к T*
        U* check_type = nullptr;
        T* type_safe = static_cast<T*>(check_type);
        (void)type_safe; // подавляем предупреждение о неиспользуемой переменной

        // UnqPtr<U> и UnqPtr<T> — это разные типы для компилятора,
        // но в памяти они оба состоят ровно из одного сырого указателя.
        // Поэтому каст через reinterpret_cast здесь абсолютно валиден и эффективен.
        master = reinterpret_cast<UnqPtr<T>*>(other.master);
        ref_count = other.ref_count;

        if (ref_count) {
            (*ref_count)++;
        }
    }

    // Оператор присваивания
    ShrdPtr& operator=(const ShrdPtr& other) {
        if (this != &other) {
            release(); // Отпускаем старый объект

            master = other.master;
            ref_count = other.ref_count;
            if (ref_count) {
                (*ref_count)++;
            }
        }
        return *this;
    }

    // Деструктор
    ~ShrdPtr() {
        release();
    }

    // Доступ (цепочка: ShrdPtr -> UnqPtr -> T)
    T& operator*() const { return **master; }
    T* operator->() const { return master->get(); }

    // Для тестов
    size_t use_count() const { return ref_count ? *ref_count : 0; }
};
*/