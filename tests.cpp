#include <gtest/gtest.h>
#include <utility>
#include "SmartPtrs.h"

struct Tracker {
    static int alive;
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

// Тесты для UniqPtr<T>
TEST(UniqPtrTest, BasicAllocationAndCleanup) {
Tracker::alive = 0;
{
UniqPtr<Tracker> p(new Tracker(10));
EXPECT_EQ(Tracker::alive, 1);
EXPECT_EQ(p->value, 10);
}
EXPECT_EQ(Tracker::alive, 0);
}

TEST(UniqPtrTest, DeepCopyAndAssignment) {
Tracker::alive = 0;
{
UniqPtr<Tracker> p1(new Tracker(10));
UniqPtr<Tracker> p2 = p1; // Глубокое копирование
EXPECT_EQ(Tracker::alive, 2);
EXPECT_NE(p1.Get(), p2.Get());

p2 = p1;
EXPECT_EQ(Tracker::alive, 2);

p1 = p1;
EXPECT_EQ(Tracker::alive, 2);
}
EXPECT_EQ(Tracker::alive, 0);
}

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
// После выхода из блока объект все еще жив, т к мы сделали release()
EXPECT_EQ(Tracker::alive, 1);
delete raw; // Удаляем вручную
EXPECT_EQ(Tracker::alive, 0);
}


// Тесты для массивов UniqPtr<T[]>
TEST(UniqPtrArrayTest, ArrayCleanupAndCopy) {
Tracker::alive = 0;
{
UniqPtr<Tracker[]> arr1(new Tracker[5], 5);
EXPECT_EQ(Tracker::alive, 5);

UniqPtr<Tracker[]> arr2 = arr1; // Глубокое копирование массива
EXPECT_EQ(Tracker::alive, 10);
}

EXPECT_EQ(Tracker::alive, 0);
}

// Тесты для ShrdPtr<T> и ShrdPtr<T[]>
TEST(ShrdPtrTest, RefCountingAndCleanup) {
Tracker::alive = 0;
{
ShrdPtr<Tracker> p1(new Tracker(42));
EXPECT_EQ(Tracker::alive, 1);
EXPECT_EQ(p1.use_count(), 1);

{
ShrdPtr<Tracker> p2 = p1;
ShrdPtr<Tracker> p3;
p3 = p2;
EXPECT_EQ(Tracker::alive, 1);
EXPECT_EQ(p1.use_count(), 3);
}

EXPECT_EQ(Tracker::alive, 1);
EXPECT_EQ(p1.use_count(), 1);
}
EXPECT_EQ(Tracker::alive, 0);
}

TEST(ShrdPtrTest, SubtypingCleanup) {
Tracker::alive = 0;
{
ShrdPtr<DerivedTracker> derived(new DerivedTracker(99));
ShrdPtr<Tracker> base = derived;
EXPECT_EQ(Tracker::alive, 1);
EXPECT_EQ(base.use_count(), 2);
}
EXPECT_EQ(Tracker::alive, 0);
}

TEST(ShrdPtrArrayTest, ArrayCleanup) {
Tracker::alive = 0;
{
ShrdPtr<Tracker[]> arr1(new Tracker[4]);
ShrdPtr<Tracker[]> arr2 = arr1;
EXPECT_EQ(Tracker::alive, 4);
EXPECT_EQ(arr1.use_count(), 2);
}
EXPECT_EQ(Tracker::alive, 0);
}