#include <gtest/gtest.h> // Фреймворк GoogleTest (макросы TEST, EXPECT_EQ и т.д.)
#include <utility>       // std::move — для тестирования семантики перемещения
#include <type_traits>   // std::is_copy_constructible_v — проверка запрета копирования на этапе компиляции
#include "UniqPtr.h"
#include "ShrdPtr.h"

struct Tracker {
    static int alive; // Общий счётчик для всех объектов Tracker
    int value;

    Tracker(int v = 0) : value(v) { ++alive; }
    Tracker(const Tracker& other) : value(other.value) { ++alive; }

    virtual ~Tracker() { --alive; }
};

int Tracker::alive = 0;

// Наследник для проверки подтипизации
struct DerivedTracker : public Tracker {
    DerivedTracker(int v = 0) : Tracker(v) {}
};

TEST(UniqPtrTest, BasicAllocationAndCleanup) {
    Tracker::alive = 0;
    {
        UniqPtr<Tracker> p(new Tracker(10));
        EXPECT_EQ(Tracker::alive, 1);
        EXPECT_EQ(p->value, 10);
    }
    EXPECT_EQ(Tracker::alive, 0);
}

//проверка, что копировать UniqPtr нельзя
TEST(UniqPtrTest, CopyIsDeleted) {
    static_assert(!std::is_copy_constructible_v<UniqPtr<Tracker>>);
    static_assert(!std::is_copy_assignable_v<UniqPtr<Tracker>>);
    static_assert(!std::is_copy_constructible_v<UniqPtr<Tracker[]>>);
    static_assert(!std::is_copy_assignable_v<UniqPtr<Tracker[]>>);
}

// Проверка семантики перемещения
TEST(UniqPtrTest, MoveSemantics) {
    Tracker::alive = 0;
    {
        UniqPtr<Tracker> p1(new Tracker(5));
        UniqPtr<Tracker> p2 = std::move(p1);

        EXPECT_EQ(Tracker::alive, 1);
        EXPECT_FALSE(static_cast<bool>(p1));
        EXPECT_TRUE(static_cast<bool>(p2));
    }
    EXPECT_EQ(Tracker::alive, 0);
}

// Проверка подтипизации для UniqPtr
TEST(UniqPtrTest, SubtypingMoveCleanup) {
    Tracker::alive = 0;
    {
        UniqPtr<DerivedTracker> derived(new DerivedTracker(77));
        UniqPtr<Tracker> base = std::move(derived);

        EXPECT_EQ(Tracker::alive, 1);
        EXPECT_FALSE(static_cast<bool>(derived));
        EXPECT_EQ(base->value, 77);
    }
    EXPECT_EQ(Tracker::alive, 0);
}

// Проверка ручного управления владением: Reset() и Release()
TEST(UniqPtrTest, ResetAndRelease) {
    Tracker::alive = 0;
    Tracker* raw = nullptr;
    {
        UniqPtr<Tracker> p(new Tracker(1));
        p.Reset(new Tracker(2));
        EXPECT_EQ(Tracker::alive, 1);
        EXPECT_EQ(p->value, 2);

        raw = p.Release();
        EXPECT_FALSE(static_cast<bool>(p));
    }
    EXPECT_EQ(Tracker::alive, 1);
    delete raw; // Удаляем вручную
    EXPECT_EQ(Tracker::alive, 0);
}

//проверка выделения массива, перемещения и корректного delete[]
TEST(UniqPtrArrayTest, ArrayCleanupAndMove) {
    Tracker::alive = 0;
    {
        UniqPtr<Tracker[]> arr1(new Tracker[5], 5);
        EXPECT_EQ(Tracker::alive, 5);
        EXPECT_EQ(arr1.Size(), 5);

        UniqPtr<Tracker[]> arr2 = std::move(arr1);
        EXPECT_EQ(Tracker::alive, 5);
        EXPECT_FALSE(static_cast<bool>(arr1));
        EXPECT_EQ(arr1.Size(), 0);
        EXPECT_EQ(arr2.Size(), 5);
    }
    EXPECT_EQ(Tracker::alive, 0);
}

// Проверка подсчёта ссылок
TEST(ShrdPtrTest, RefCountingAndCleanup) {
    Tracker::alive = 0;
    {
        ShrdPtr<Tracker> p1(new Tracker(42));
        EXPECT_EQ(Tracker::alive, 1);
        EXPECT_EQ(p1.Use_count(), 1);
        {
            ShrdPtr<Tracker> p2 = p1;
            ShrdPtr<Tracker> p3;
            p3 = p2;
            EXPECT_EQ(Tracker::alive, 1);
            EXPECT_EQ(p1.Use_count(), 3);
        }

        EXPECT_EQ(Tracker::alive, 1);
        EXPECT_EQ(p1.Use_count(), 1);
    }
    EXPECT_EQ(Tracker::alive, 0);
}

// Проверка подтипизации для ShrdPtr
TEST(ShrdPtrTest, SubtypingCleanup) {
    Tracker::alive = 0;
    {
        ShrdPtr<DerivedTracker> derived(new DerivedTracker(99));
        ShrdPtr<Tracker> base = derived; // Делят один и тот же ControlBlock
        EXPECT_EQ(Tracker::alive, 1);
        EXPECT_EQ(base.Use_count(), 2);
    }
    EXPECT_EQ(Tracker::alive, 0);
}

// Проверка разделяемого владения для массива
TEST(ShrdPtrArrayTest, ArrayCleanup) {
    Tracker::alive = 0;
    {
        ShrdPtr<Tracker[]> arr1(new Tracker[4]);
        ShrdPtr<Tracker[]> arr2 = arr1;
        EXPECT_EQ(Tracker::alive, 4);
        EXPECT_EQ(arr1.Use_count(), 2);
    }
    EXPECT_EQ(Tracker::alive, 0);
}