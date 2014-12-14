#ifndef GAUSSIAN_H
#define GAUSSIAN_H

#include <cassert>
#include <cmath>

#include <opencv2/core/core.hpp>

void grayscaleGaussianBlur(cv::Mat &source, cv::Mat &destination, int sizeX, int sizeY = -1);

#endif // GAUSSIAN_H