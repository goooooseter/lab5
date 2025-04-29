#include "blur.hpp"
#include <iostream>
#include <chrono>



int main() {
    try {
        int width, height;
        auto image = loadImage("input.jpg", width, height);
        std::cout << "Image loaded: " << width << "x" << height << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        auto seq_result = sequentialBlur(image);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> seq_duration = end - start;
        std::cout << "Sequential blur: " << seq_duration.count() << " seconds" << std::endl;
        saveImage("sequential_blur.jpg", seq_result);

        start = std::chrono::high_resolution_clock::now();
        auto thread_result = parallelBlurThreads(image, 4);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> thread_duration = end - start;
        std::cout << "Thread blur (4 threads): " << thread_duration.count() << " seconds" << std::endl;
        saveImage("thread_blur.jpg", thread_result);

        std::cout << "\nTesting atomic operations:" << std::endl;
        testAtomicOperations();

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}