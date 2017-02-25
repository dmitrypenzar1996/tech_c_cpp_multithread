#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <skiplist/skiplist.h>
#include <iostream>
using namespace std;

SkipList<int, string, 8> sk;

TEST(SkipListTest, Empty) {
  ASSERT_EQ(nullptr, sk.Get(100));
  ASSERT_EQ(sk.cend(), sk.cbegin())  << "Begin iterator fails";
  ASSERT_EQ(sk.cend(), sk.cfind(10)) << "Find iterator fails";
  // test dump
  sk.dump("testEmpty.dot");
}

TEST(SkipListTest, SimplePut) {
  SkipList<int, string, 8> sk;

  const std::string *pOld = sk.Put(10, "test");
  ASSERT_EQ(nullptr, pOld);

  pOld = sk[10];
  ASSERT_NE(nullptr, pOld)         << "Value found";
  ASSERT_EQ(string("test"), *pOld) << "Value is correct";

  pOld = sk.Get(10);
  ASSERT_NE(nullptr, pOld)         << "Value found";
  ASSERT_EQ(string("test"), *pOld) << "Value is correct";

  Iterator<int,std::string> it = sk.cbegin();
  ASSERT_NE(sk.cend(), it)              << "Iterator is not empty";
  ASSERT_EQ(10, it.key())               << "Iterator key is correct";
  ASSERT_EQ(string("test"), it.value()) << "Iterator value is correct";
  ASSERT_EQ(string("test"), *it)        << "Iterator value is correct";
  sk.dump("testSimplePut.dot");
}

TEST(SkipListTest, PutIfAbsent)
{
    SkipList<int, string, 8> sk;
    const std::string *pOld = sk.Put(10, "test");
    ASSERT_EQ(nullptr, pOld);
    pOld = sk.Put(10, "newtest");
    ASSERT_EQ("test", *pOld);
    pOld = sk.PutIfAbsent(10, "test");
    ASSERT_EQ("newtest", *pOld);
    pOld = sk.Delete(10);
    ASSERT_EQ("newtest", *pOld);
    pOld = sk.Get(10);
    ASSERT_EQ(nullptr, pOld);
    pOld = sk.PutIfAbsent(10, "test");
    ASSERT_EQ(nullptr, pOld);
    pOld = sk.Get(10);
    ASSERT_EQ("test", *pOld);
    sk.dump("testPutIfAbsent.dot");
}


TEST(SkipListTest, SortedOrder)
{
    SkipList<int, string, 8> sk;
    int arr[10] = {1, 9, 10, 2, 3, 4, 7, 169, 0, 11};
    int arr_2[10] = {1, 9, 10, 2, 3, 4, 7, 169, 0, 11};
    std::sort(std::begin(arr_2), std::end(arr_2));

    for(int i = 0; i < 10; ++i)
    {
        sk.Put(arr[i], "test_" + std::to_string(arr[i]));
    }

    Iterator<int,std::string> it = sk.cbegin();
    for(int i = 0; i < 10; ++i)
    {
        ASSERT_EQ(it.key(), arr_2[i]);
        ++it;
    }
    ASSERT_EQ(sk.cend(), it);
    sk.dump("testSortedOrder.dot");
}
