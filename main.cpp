#include <iostream>
#include <thread>
#include <bitset>
#include <chrono>

#include "MemoryManager.h"
#include "PoolAllocator.h"
#include "StackAllocator.h"
#include "Vector.h"
#include "Scope.h"
#include "Ref.h"

using namespace ME;

class timer
{
public:
    timer()
    {
        start = std::chrono::high_resolution_clock::now();
    }
    ~timer()
    {
        std::cout << std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start.time_since_epoch()).time_since_epoch().count() << std::endl;
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

int main()
{

    ME_MEMINIT();
    {
        size_t *i = alloc<size_t>(19);
        for (size_t l = 0; l < 19; l++)
            i[l] = l;
        Scope<int> k = CreateScope<int>(7);
        Ref<size_t> m = CreateRef<size_t>(9);
        Ref<size_t> n = m;
        Vector<int> vec;
        vec.push_back(1);
        vec.emplace_back(2);
        vec.push_back(3);
        int *j = allocon<int>(1);
        vec.push_back(4);
        vec.push_back(5);
        std::cout << *n << std::endl;
    }
    ME_MEMCLEAR();
}