#include <iostream>
#include "containers/threadSafe/Queue.hpp"
#include "containers/threadSafe/lockFree/Queue.hpp"
#include "containers/threadSafe/lockFree/Stack.hpp"
#include "containers/threadSafe/lockFree/StackLT.hpp"

#include <future>
#include <thread>
#include <vector>

namespace stack_example
{

// non_std::containers::thread_safe::Queue<int> tsContainer;
non_std::containers::thread_safe::lock_free::Queue<int> tsContainer;
// non_std::containers::thread_safe::lock_free::Stack<int> tsContainer;
// non_std::containers::thread_safe::lock_free::StackLT<int> tsContainer;

template <template<typename> typename T, typename TIN>
auto print(typename T<TIN> val)
{
    if (!val)
    {
        std::cout << "none" << std::endl;
        return;
    }
    std::cout << *val << std::endl;
}

long long test()
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

auto example()
{
    for (int i = 0; i < 1000; ++i)
    {
        std::vector<std::future<long long>> asyncs;
        long long sum = 0;
        for (int i = 0; i < 12; ++i)
        {
            asyncs.push_back(std::async(test));
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

}  // namespace stack_example

int main()
{
    // stack_example::example();

    return 0;
}
