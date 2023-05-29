#include "Memory/MemoryManager.h"

#include <iostream>
#include <chrono>

int main() {
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        {
            // Performing ~38MBs of allocations using STL
            for (int i = 0; i < 10000000; i++)
                std::free(malloc(sizeof(size_t)));
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        std::cout << "Benchmark duration: " << duration.count() << " seconds" << std::endl;
    }
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        {
            InitAllocator();
            {
                // Performing ~38MBs of allocations using MTL
                for (int i = 0; i < 10000000; i++)
                    ME::dealloc(ME::alloc<size_t>());
            }
            DeInitAllocator();
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        std::cout << "Benchmark duration: " << duration.count() << " seconds" << std::endl;
    }
    return 0;
}
