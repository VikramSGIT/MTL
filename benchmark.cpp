#include "Memory/MemoryManager.h"

#include <iostream>
#include <chrono>

#include "Vector.h"
#ifdef ME_VECTOR
#include <vector>
#endif

template <typename T>
class CustomAllocator {
public:
    using value_type = T;

    CustomAllocator() noexcept = default;

    template <typename U>
    CustomAllocator(const CustomAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        return ME::alloc<T>(n);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        ME::dealloc(p);
    }
};

int main() {
    
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        {
            InitAllocator();
            {
                for (int i = 0; i < 10000000; i++)
                    ME::allocon<size_t>(i);
            }
            DeInitAllocator();
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        std::cout << "MTL Allocation Benchmark duration: " << duration.count() * 1000 << " ms" << std::endl;
    }
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        {
            for (int i = 0; i < 10000000; i++)
               *reinterpret_cast<size_t*>(malloc(sizeof(size_t))) = i;
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        std::cout << "STL Allocation Benchmark duration: " << duration.count() * 1000 << " ms" << std::endl;
    }
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        {
            InitAllocator();
            {
                for (int time = 100; time >= 0; time--) {
                    ME::vector<size_t> vec;
                    for (int i = 0; i < 100; i++)
                        vec.push_back(i);
                }
            }
            DeInitAllocator();
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        std::cout << "MTL Vector PushBack duration: " << duration.count() * 1000 << " ms" << std::endl;
    }
    
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        {
            {
                for (int time = 100; time >= 0; time--) {
                    std::vector<size_t> vec;
                    for (int i = 0; i < 100; i++)
                        vec.push_back(i);
                }
            }
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        std::cout << "STL Vector PushBack duration: " << duration.count() * 1000 << " ms" << std::endl;
    }
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        {
            InitAllocator();
            {
                for (int time = 100; time >= 0; time--) {
                    std::vector<size_t, CustomAllocator<size_t>> vec;
                    for (int i = 0; i < 100; i++)
                        vec.push_back(i);
                }
            }
            DeInitAllocator();
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                                                                                                                                                   
        std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        std::cout << "STL Vector modified for MTL Allocators PushBack Benchmark duration: " << duration.count() * 1000 << " ms" << std::endl;
    }
    
    return 0;
}
