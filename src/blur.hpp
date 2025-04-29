#ifndef BLUR_HPP
#define BLUR_HPP

#include <vector>
#include <string>

struct Color {
    unsigned char r, g, b;
};

std::vector<std::vector<Color>> loadImage(const std::string& filename, int& width, int& height);
void saveImage(const std::string& filename, const std::vector<std::vector<Color>>& image);

std::vector<std::vector<Color>> sequentialBlur(const std::vector<std::vector<Color>>& image);
std::vector<std::vector<Color>> parallelBlurThreads(const std::vector<std::vector<Color>>& image, int num_threads);

void testAtomicOperations();

#endif // BLUR_HPP