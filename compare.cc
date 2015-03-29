#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <ctime>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

int main(int argc, char** argv) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <first disparity map image> <second disparity map image>" << std::endl;
        return -1;
    }

    cv::Mat firstImage;
    cv::Mat secondImage;
    firstImage = cv::imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    secondImage = cv::imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

    if (!firstImage.data || !secondImage.data) {
        std::cerr <<  "Could not open or find one of the images!" << std::endl;
        return -1;
    }

    if (firstImage.rows != secondImage.rows || firstImage.cols != secondImage.cols) {
        std::cerr << "Error: The images have different dimensions!" << std::endl;
        return -2;
    }

    unsigned long counter, sum;
    counter = sum = 0;
    for (int row = firstImage.rows/10; row < firstImage.rows && row < secondImage.rows; ++row) {
        for (int col = 0; col < firstImage.cols && col < secondImage.cols; ++col) {
            counter++;
            sum += abs(firstImage.at<uchar>(row, col) - secondImage.at<uchar>(row, col));
        }
    }

    std::cout << 1 - (sum / (counter * 255.0f));
    return 0;
}