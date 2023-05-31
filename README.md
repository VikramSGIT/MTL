# Mars Templated Libraries (MTL)

<p align="center"><img width="500" alt="MarsUtilies" src="https://github.com/VikramSGIT/MTL/assets/49726816/957269aa-20fd-4aa8-9a82-d7b2e06ae8d1"></p>

Follow us on:

 [<img src="https://cdn.cdnlogo.com/logos/i/4/instagram.svg" height="33" alt="Instagram">](https://instagram.com/uchiha_coder?igshid=NGExMmI2YTkyZg==)
 &nbsp;&nbsp;
[<img src="https://assets-global.website-files.com/6257adef93867e50d84d30e2/636e0a6a49cf127bf92de1e2_icon_clyde_blurple_RGB.png" height="30" alt="Discord">](https://discord.gg/xTdkh8cv)

## Introduction

Mars Templated Libraries (MTL) is a collection of templated libraries in C++ designed to provide convenient and efficient data structures and utilities. Built on top of a highly optimized pooled allocator, MTL offers blazing-fast allocations and deallocations, which are 2X faster than traditional methods. This speed improvement extends to the utilities and data structures provided by MTL, making them ideal for larger applications with complex memory management requirements. Additionally, MTL supports encryption of the underlying memory and includes caching mechanisms for frequently used memory, further enhancing security and performance.

Benchmark results:
![Benchmark Results](https://github.com/VikramSGIT/MTL/assets/49726816/f3b0bc13-c810-4ab5-860a-11f1943734d9)

The libraries in MTL, including String, Vector, Array, Smart Pointers, Stack Allocator, and Bitmap Allocator, provide robust and efficient solutions for various programming tasks. Whether you are working with strings, arrays, or managing memory resources, MTL's templated libraries offer high-performance and reliable functionality.

## Features

### String

The String library in MTL provides a comprehensive set of features for string manipulation. It offers functions for concatenation, substring extraction, searching, replacing, and many more. With MTL's String, you can handle strings easily and efficiently.

### Vector

The Vector library is a dynamic array implementation that allows you to store and manipulate a collection of elements. It provides methods for adding, removing, and accessing elements efficiently. MTL's Vector is designed to be a flexible and efficient container for various use cases.

### Array

MTL's Array library provides a fixed-size array implementation, ideal for scenarios where a fixed number of elements need to be stored and accessed. It offers fast and direct access to elements without the overhead of dynamic resizing.

### Smart Pointers

MTL includes two smart pointer implementations: `Ref` and `Scope`

- `Ref` is a reference counting smart pointer that automatically manages the lifetime of objects and frees the memory when there are no more references to it.
- `Scope` is a smart pointer that manages ownership and ensures that the object is automatically freed when it goes out of scope.

These smart pointers provide memory management and ownership semantics, making it easier to handle resources and prevent memory leaks.

### Pooled Allocator

MTL utilizes a pooled allocator, a memory management technique that improves memory allocation and deallocation performance. It efficiently allocates and deallocates memory blocks of fixed sizes, reducing the overhead of frequent memory allocations. The pooled allocator in MTL offers **2x times faster allocation compared to traditional allocation (new or malloc)** methods, making it ideal for larger applications with complex memory management requirements.

### Stack Allocator

The Stack Allocator in MTL provides a fast and efficient memory allocation strategy. It works in a last-in, first-out (LIFO) manner, allowing for quick allocation and deallocation of memory blocks. This allocator is particularly useful for scenarios where temporary memory needs to be allocated and released in a predictable order.

### Bitmap Allocator

MTL offers a bitmap allocator, which manages memory blocks using a bitmap representation. It allows for efficient allocation and deallocation of memory blocks of varying sizes, reducing fragmentation and improving memory utilization.

MTL also supports encryption of the underlying memory, providing an additional layer of security for sensitive data. Additionally, MTL includes caching mechanisms for frequently used memory, improving performance and reducing memory access latency.

## Usage

To use the MTL libraries in your C++ projects, follow these steps:

1. Clone or download the MTL repository from [here](https://github.com/VikramSGIT/MTL).
2. Include the necessary MTL headers in your source files.
3. Link your project with the [MTL library](https://github.com/VikramSGIT/MTL/releases/tag/Release).
4. Use the MTL classes and functions as described in the [Documentation]().

Refer to the documentation and examples in the repository for detailed instructions on using each library.

## Contributing

Contributions to MTL are welcome! If you find a bug, have a suggestion, or want to contribute new features, please follow the guidelines outlined in the CONTRIBUTING.md file in the repository.

## License

MTL is released under the [LICENSE](https://github.com/VikramSGIT/MTL/blob/master/LICENSE), allowing you to use, modify, and distribute the library. Make sure to review the license file for more details.

## Acknowledgments

MTL was developed by Uchiha and is inspired by various open-source libraries and best practices in C++ programming. We would like to thank the contributors and the C++ community for their continuous support and feedback.

## Contact

If you have any questions, feedback, or inquiries regarding MTL, feel free to reach out (socials are on the top of readme)


Happy coding with Mars Templated Libraries (MTL)!
