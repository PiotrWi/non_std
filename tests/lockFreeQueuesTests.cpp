#include "lockFreeQueuesTests.hpp"

#include <iostream>
#include "containers/threadSafe/Queue.hpp"
#include "containers/threadSafe/lockFree/Queue.hpp"
#include "containers/threadSafe/lockFree/Stack.hpp"
#include "containers/threadSafe/lockFree/StackLT.hpp"

#include <future>
#include <thread>
#include <vector>

namespace test::lock_free_structures
{

// non_std::containers::thread_safe::Queue<int> tsContainer;
non_std::containers::thread_safe::lock_free::Queue<int> tsContainer;
// non_std::containers::thread_safe::lock_free::Stack<int> tsContainer;
// non_std::containers::thread_safe::lock_free::StackLT<int> tsContainer;

template <typename T>
auto print(T val)
{
    if (!val)
    {
        std::cout << "none" << std::endl;
        return;
    }
    std::cout << *val << std::endl;
}

long long testPushPop()
{
    long long sum = 0;
    for (int i = 0; i < 1000; ++i)
    {
        tsContainer.push(i);
    }
    for (int i = 0; i < 1000; ++i)
    {
        auto j = tsContainer.pop();
        if (j)
        {
            sum += *j;
        }
    }
    return sum;
}

void test()
{
    for (int i = 0; i < 1000; ++i)
    {
        std::vector<std::future<long long>> asyncs;
        long long sum = 0;
        for (int i = 0; i < 12; ++i)
        {
            asyncs.push_back(std::async(testPushPop));
        }
        for (int i = 0; i < 12; ++i)
        {
            sum += asyncs[i].get();
        }
        test();
        while (auto j = tsContainer.pop())
        {
            sum += *j;
        }
        std::cout << sum << std::endl;
    }
}

}  // namespace test::lock_free_structures