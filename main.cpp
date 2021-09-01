#include <iostream>
#include "FibonacciHeap.h"

int main()
{
    non_std::fibonacci::Heap<float> heap;

    for (int i = 0; i < 100; ++i)
    {
        heap.insert(i);
        std::cout << heap.top() << std::endl;
    }
    for (int i = 0; i < 100; ++i)
    {
        std::cout << heap.top() << std::endl;
        heap.pop();
    }

    return 0;
}
