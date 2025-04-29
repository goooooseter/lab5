#include "blur.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

std::mutex cout_mutex;

std::vector<std::vector<Color>> loadImage(const std::string& filename, int& width, int& height) {
    int channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);
    if (!data) {
        throw std::runtime_error("Failed to load image");
    }

    std::vector<std::vector<Color>> image(height, std::vector<Color>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;
            image[y][x] = { data[index], data[index + 1], data[index + 2] };
        }
    }

    stbi_image_free(data);
    return image;
}

void saveImage(const std::string& filename, const std::vector<std::vector<Color>>& image) {
    int width = image[0].size();
    int height = image.size();

    std::vector<unsigned char> data(width * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;
            data[index] = image[y][x].r;
            data[index + 1] = image[y][x].g;
            data[index + 2] = image[y][x].b;
        }
    }

    stbi_write_png(filename.c_str(), width, height, 3, data.data(), width * 3);
}

Color blurPixel(const std::vector<std::vector<Color>>& image, int x, int y, int width, int height) {
    int radius = 2;
    int count = 0;
    int r = 0, g = 0, b = 0;

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                r += image[ny][nx].r;
                g += image[ny][nx].g;
                b += image[ny][nx].b;
                count++;
            }
        }
    }

    return {
        static_cast<unsigned char>(r / count),
        static_cast<unsigned char>(g / count),
        static_cast<unsigned char>(b / count)
    };
}

std::vector<std::vector<Color>> sequentialBlur(const std::vector<std::vector<Color>>& image) {
    int height = image.size();
    int width = image[0].size();

    std::vector<std::vector<Color>> result(height, std::vector<Color>(width));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            result[y][x] = blurPixel(image, x, y, width, height);
        }
    }

    return result;
}

std::vector<std::vector<Color>> parallelBlurThreads(const std::vector<std::vector<Color>>& image, int num_threads) {
    int height = image.size();
    int width = image[0].size();

    std::vector<std::vector<Color>> result(height, std::vector<Color>(width));
    std::vector<std::thread> threads;

    auto processPart = [&](int start_y, int end_y) {
        for (int y = start_y; y < end_y; ++y) {
            for (int x = 0; x < width; ++x) {
                result[y][x] = blurPixel(image, x, y, width, height);
            }
        }
        };

    int rows_per_thread = height / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        int start_y = i * rows_per_thread;
        int end_y = (i == num_threads - 1) ? height : start_y + rows_per_thread;
        threads.emplace_back(processPart, start_y, end_y);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}

void testAtomicOperations() {
    const int iterations = 1000000;
    const int num_threads = 4;

    {
        int counter = 0;
        std::mutex counter_mutex;
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < iterations; ++j) {
                    std::lock_guard<std::mutex> lock(counter_mutex);
                    counter++;
                }
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Mutex version: " << duration.count() << " seconds, counter = " << counter << std::endl;
    }

    {
        std::atomic<int> counter(0);
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < iterations; ++j) {
                    counter++;
                }
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Atomic version: " << duration.count() << " seconds, counter = " << counter << std::endl;
    }
}