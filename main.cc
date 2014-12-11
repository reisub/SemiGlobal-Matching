#include <iostream>
#include <algorithm>
#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define SMALL_PENALTY 1
#define LARGE_PENALTY 5

unsigned int calculatePixelCostOneWayBT(int row, int leftCol, int rightCol, const cv::Mat &leftImage, const cv::Mat &rightImage) {

    char leftValue, rightValue, beforeRightValue, afterRightValue, rightValueMinus, rightValuePlus, rightValueMin, rightValueMax;

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

int calculatePixelCostBT(int row, int leftCol, int rightCol, const cv::Mat &leftImage, const cv::Mat &rightImage) {
    return std::min(calculatePixelCostOneWayBT(row, leftCol, rightCol, leftImage, rightImage),
        calculatePixelCostOneWayBT(row, rightCol, leftCol, rightImage, leftImage));
}

void calculatePixelCost(cv::Mat &firstImage, cv::Mat &secondImage, int disparityRange, unsigned short ***C) {
    for (int row = 0; row < firstImage.rows; ++row) {
        for (int col = 0; col < firstImage.cols; ++col) {
            for (int d = 0; d < disparityRange; ++d) {
                C[row][col][d] = calculatePixelCostBT(row, col, col - d, firstImage, secondImage);
            }
        }
    }
}

void aggregateCosts(int rows, int cols, int disparityRange, unsigned short ***C, unsigned short ***S) {
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            for (int d = 0; d < disparityRange; ++d) {
                // TODO implement!
                S[row][col][d] = C[row][col][d];
            }
        }
    }
}

void computeDisparity(unsigned short ***S, int rows, int cols, int disparityRange, cv::Mat &disparityMap) {
    unsigned int disparity, minCost;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            minCost = 65535;
            for (int d = 0; d < disparityRange; ++d) {
                if(minCost > S[row][col][d]) {
                    minCost = S[row][col][d];
                    disparity = d;
                }
            }
            disparityMap.at<uchar>(row, col) = disparity;
        }
    }
}

void showDisparityMap(cv::Mat &disparityMap, int disparityRange) {
    double factor = 256.0 / disparityRange;
    for (int row = 0; row < disparityMap.rows; ++row) {
        for (int col = 0; col < disparityMap.cols; ++col) {
            disparityMap.at<uchar>(row, col) *= factor;
        }
    }
    cv::namedWindow("Disparity map", CV_WINDOW_AUTOSIZE);
    cv::imshow("Disparity map", disparityMap);
    cv::waitKey(0);
}


int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <left image> <right image>" << std::endl;
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

    unsigned int disparityRange = 32;
    unsigned short ***C; // pixel cost array W x H x D
    unsigned short ***S; // aggregated cost array W x H x D

    // allocate cost arrays
    C = new unsigned short**[firstImage.rows];
    S = new unsigned short**[firstImage.rows];
    for (int row = 0; row < firstImage.rows; ++row) {
        C[row] = new unsigned short*[firstImage.cols];
        S[row] = new unsigned short*[firstImage.cols]();
        for (int col = 0; col < firstImage.cols; ++col) {
            C[row][col] = new unsigned short[disparityRange];
            S[row][col] = new unsigned short[disparityRange](); // initialize to 0
        }
    }

    calculatePixelCost(firstImage, secondImage, disparityRange, C);
    aggregateCosts(firstImage.rows, firstImage.cols, disparityRange, C, S);

    cv::Mat disparityMap = cv::Mat(cv::Size(firstImage.cols, firstImage.rows), CV_8UC1, cv::Scalar::all(0));

    computeDisparity(S, firstImage.rows, firstImage.cols, disparityRange, disparityMap);
    showDisparityMap(disparityMap, disparityRange);

    return 0;
}