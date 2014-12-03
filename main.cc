#include <iostream>
#include <algorithm>
#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define SMALL_PENALTY 1.0
#define LARGE_PENALTY 3.0

unsigned int calculatePixelCostOneWayBT(unsigned int row, unsigned int leftCol, unsigned int rightCol, const cv::Mat &leftImage, const cv::Mat &rightImage) {

    unsigned char leftValue, rightValue, beforeRightValue, afterRightValue, rightValueMinus, rightValuePlus, rightValueMin, rightValueMax;

    leftValue = leftImage.at<uchar>(row, leftCol);
    rightValue = rightImage.at<uchar>(row, rightCol);

    if (rightCol > 0) {
        beforeRightValue = rightImage.at<uchar>(row, rightCol - 1);
    } else {
        beforeRightValue = rightValue;
    }

    if (rightCol + 1 < rightImage.cols) {
        afterRightValue = rightImage.at<uchar>(row, rightCol + 1);
    } else {
        afterRightValue = rightValue;
    }

    rightValueMinus = round((rightValue + beforeRightValue) / 2.f);
    rightValuePlus = round((rightValue + afterRightValue) / 2.f);

    rightValueMin = std::min(rightValue, std::min(rightValueMinus, rightValuePlus));
    rightValueMax = std::max(rightValue, std::max(rightValueMinus, rightValuePlus));

    return std::max(0, std::max((leftValue - rightValueMax), (rightValueMin - leftValue)));
}

unsigned int calculatePixelCostBT(unsigned int row, unsigned int leftCol, unsigned int rightCol, const cv::Mat &leftImage, const cv::Mat &rightImage) {
    // TODO memoization using hash map
    return std::min(calculatePixelCostOneWayBT(row, leftCol, rightCol, leftImage, rightImage),
        calculatePixelCostOneWayBT(row, rightCol, leftCol, rightImage, leftImage));
}

double calculateGlobalCost(unsigned int row, unsigned int leftCol, unsigned int rightCol, const cv::Mat &leftImage, const cv::Mat &rightImage) {
    double globalCost = 0.0;

    globalCost += calculatePixelCostBT(row, leftCol, rightCol, leftImage, rightImage);
    // TODO calculate the global cost using the 8-way neighborhood of the matching element

    return globalCost;
}

unsigned int calculateDisparity(unsigned int row, unsigned int col, unsigned int disparityRange, const cv::Mat &leftImage, const cv::Mat &rightImage) {
    int disparity = 0;
    double minGlobalCost = 1e10;
    double globalCost;

    unsigned int startCol = (col - disparityRange >= 0) ? (col - disparityRange) : 0;
    unsigned int endCol = (col + disparityRange < leftImage.cols) ? (col + disparityRange) : (leftImage.cols - 1);

    for (unsigned int currentCol = startCol; currentCol <= endCol; ++currentCol) {
        double globalCost = calculateGlobalCost(row, col, currentCol, leftImage, rightImage);
        if (globalCost < minGlobalCost) {
            minGlobalCost = globalCost;
            disparity = currentCol - col;
        }
    }

    return abs(disparity);
}

void calculateDisparityMap(cv::Mat &leftImage, cv::Mat &rightImage, cv::Mat &disparityMap, unsigned int disparityRange) {
    for (unsigned int row = 0; row < leftImage.rows; ++row) {
        for (int col = 0; col < leftImage.cols; ++col) {
            unsigned char disparity = calculateDisparity(row, col, disparityRange, leftImage, rightImage);
            disparityMap.at<uchar>(row, col) = disparity * (255.0 / disparityRange);
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <first image> <second image>" << std::endl;
        return -1;
    }

    cv::Mat firstImage;
    cv::Mat secondImage;
    firstImage = cv::imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    secondImage = cv::imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

    if(!firstImage.data || !secondImage.data) {
        std::cerr <<  "Could not open or find one of the images!" << std::endl;
        return -1;
    }

    cv::Mat disparityMap = cv::Mat(cv::Size(firstImage.cols, firstImage.rows), CV_8UC1, cv::Scalar::all(0));
    unsigned int disparityRange = 32;

    calculateDisparityMap(firstImage, secondImage, disparityMap, disparityRange);

    cv::namedWindow("Disparity map", CV_WINDOW_AUTOSIZE);
    cv::imshow("Disparity map", disparityMap);
    cv::waitKey(0);

    return 0;
}