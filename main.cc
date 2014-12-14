#include <iostream>
#include <algorithm>
#include <cmath>
#include <ctime>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "gaussian.h"

#define BLUR_RADIUS 3
#define PATH_COUNT 8
#define MAX_SHORT 65535
#define SMALL_PENALTY 5
#define LARGE_PENALTY 20
#define DEBUG false

void printArray(unsigned short ***array, int rows, int cols, int depth) {
    for (int d = 0; d < depth; ++d) {
        std::cout << "disparity: " << d << std::endl;
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                std::cout << "\t" << array[row][col][d];
            }
            std::cout << std::endl;
        }
    }
}

unsigned short calculatePixelCostOneWayBT(int row, int leftCol, int rightCol, const cv::Mat &leftImage, const cv::Mat &rightImage) {

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

inline unsigned short calculatePixelCostBT(int row, int leftCol, int rightCol, const cv::Mat &leftImage, const cv::Mat &rightImage) {
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

inline unsigned int getPathIndex(short rowDiff, short colDiff) {
    if (rowDiff == -1 && colDiff == 0) return 0; // W
    if (rowDiff == 0 && colDiff == 1) return 1; // N
    if (rowDiff == 1 && colDiff == 0) return 2; // E
    if (rowDiff == 0 && colDiff == -1) return 3; // S
    if (rowDiff == -1 && colDiff == 1) return 4; // NW
    if (rowDiff == 1 && colDiff == 1) return 5; // NE
    if (rowDiff == 1 && colDiff == -1) return 6; // SE
    if (rowDiff == -1 && colDiff == -1) return 7; // SW

    return -1;
}

unsigned short aggregateCost(int row, int col, int d, short rowDiff, short colDiff, int rows, int cols, int disparityRange, unsigned short ***C, unsigned short ****A, unsigned short ***S, unsigned int iter) {
    unsigned short aggregatedCost = 0;
    aggregatedCost += C[row][col][d];

    if(DEBUG) {
        for (unsigned int i = 0; i < iter; ++i) {
            printf(" ");
        }
        printf("{P%d}[%d][%d](d%d)\n", getPathIndex(rowDiff, colDiff), row, col, d);
    }

    if (A[getPathIndex(rowDiff, colDiff)][row][col][d] != MAX_SHORT) {
        if(DEBUG) {
            for (unsigned int i = 0; i < iter; ++i) {
                printf(" ");
            }
            printf("{P%d}[%d][%d](d%d)-> %d<CACHED>\n", getPathIndex(rowDiff, colDiff), row, col, d, A[getPathIndex(rowDiff, colDiff)][row][col][d]);
        }
        return A[getPathIndex(rowDiff, colDiff)][row][col][d];
    }

    if (row + rowDiff < 0 || row + rowDiff >= rows || col + colDiff < 0 || col + colDiff >= cols) {
        // border
        A[getPathIndex(rowDiff, colDiff)][row][col][d] = aggregatedCost;
        if(DEBUG) {
            for (unsigned int i = 0; i < iter; ++i) {
                printf(" ");
            }
            printf("{P%d}[%d][%d](d%d)-> %d <BORDER>\n", getPathIndex(rowDiff, colDiff), row, col, d, A[getPathIndex(rowDiff, colDiff)][row][col][d]);
        }
        return A[getPathIndex(rowDiff, colDiff)][row][col][d];

        return aggregatedCost;
    }

    unsigned short minPrev, minPrevOther, prev, prevPlus, prevMinus;
    minPrev = minPrevOther = prevPlus = prevMinus = MAX_SHORT;

    for (int disp = 0; disp < disparityRange; ++disp) {
        unsigned short tmp = aggregateCost(row + rowDiff, col + colDiff, disp, rowDiff, colDiff, rows, cols, disparityRange, C, A, S, ++iter);
        if(minPrev > tmp) {
            minPrev = tmp;
        }

        if(disp == d) {
            prev = tmp;
        } else if(disp == d + 1) {
            prevPlus = tmp;
        } else if (disp == d - 1) {
            prevMinus = tmp;
        } else {
            if(minPrevOther > tmp + LARGE_PENALTY) {
                minPrevOther = tmp + LARGE_PENALTY;
            }
        }
    }

    aggregatedCost += std::min(std::min((int)prevPlus + SMALL_PENALTY, (int)prevMinus + SMALL_PENALTY), std::min((int)prev, (int)minPrevOther + LARGE_PENALTY));
    aggregatedCost -= minPrev;

    A[getPathIndex(rowDiff, colDiff)][row][col][d] = aggregatedCost;

    if(DEBUG) {
        for (unsigned int i = 0; i < iter; ++i) {
            printf(" ");
        }
        printf("{P%d}[%d][%d](d%d)-> %d<CALCULATED>\n", getPathIndex(rowDiff, colDiff), row, col, d, A[getPathIndex(rowDiff, colDiff)][row][col][d]);
    }

    return aggregatedCost;
}

void aggregateCosts(int rows, int cols, int disparityRange, unsigned short ***C, unsigned short ****A, unsigned short ***S) {
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            for (int d = 0; d < disparityRange; ++d) {
                S[row][col][d] += aggregateCost(row, col, d, 0, -1, rows, cols, disparityRange, C, A, S, 0);
                S[row][col][d] += aggregateCost(row, col, d, 0, 1, rows, cols, disparityRange, C, A, S, 0);
                S[row][col][d] += aggregateCost(row, col, d, 1, 0, rows, cols, disparityRange, C, A, S, 0);
                S[row][col][d] += aggregateCost(row, col, d, -1, 0, rows, cols, disparityRange, C, A, S, 0);
                S[row][col][d] += aggregateCost(row, col, d, 1, 1, rows, cols, disparityRange, C, A, S, 0);
                S[row][col][d] += aggregateCost(row, col, d, 1, -1, rows, cols, disparityRange, C, A, S, 0);
                S[row][col][d] += aggregateCost(row, col, d, -1, 1, rows, cols, disparityRange, C, A, S, 0);
                S[row][col][d] += aggregateCost(row, col, d, -1, -1, rows, cols, disparityRange, C, A, S, 0);
            }
        }
    }
}

void computeDisparity(unsigned short ***S, int rows, int cols, int disparityRange, cv::Mat &disparityMap) {
    unsigned int disparity, minCost;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            minCost = 65535;
            for (int d = disparityRange - 1; d >= 0; --d) {
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
    unsigned short ****A; // path cost array P x W x H x D

    clock_t begin = clock();

    std::cout << "Allocating space..." << std::endl;

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

    A = new unsigned short ***[PATH_COUNT];
    for (int p = 0; p < PATH_COUNT; ++p) {
        A[p] = new unsigned short **[firstImage.rows];
        for (int row = 0; row < firstImage.rows; ++row) {
            A[p][row] = new unsigned short*[firstImage.cols];
            for (int col = 0; col < firstImage.cols; ++col) {
                A[p][row][col] = new unsigned short[disparityRange];
                for (unsigned int d = 0; d < disparityRange; ++d) {
                    A[p][row][col][d] = MAX_SHORT;
                }
            }
        }
    }

    // std::cout << "Smoothing images..." << std::endl;
    // grayscaleGaussianBlur(firstImage, firstImage, BLUR_RADIUS);
    // grayscaleGaussianBlur(secondImage, secondImage, BLUR_RADIUS);

    std::cout << "Calculating pixel cost for the image..." << std::endl;
    calculatePixelCost(firstImage, secondImage, disparityRange, C);
    if(DEBUG) {
        printArray(C, firstImage.rows, firstImage.cols, disparityRange);
    }
    std::cout << "Aggregating costs..." << std::endl;
    aggregateCosts(firstImage.rows, firstImage.cols, disparityRange, C, A, S);

    cv::Mat disparityMap = cv::Mat(cv::Size(firstImage.cols, firstImage.rows), CV_8UC1, cv::Scalar::all(0));

    std::cout << "Computing disparity..." << std::endl;
    computeDisparity(S, firstImage.rows, firstImage.cols, disparityRange, disparityMap);

    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    printf("Done in %.2lf seconds.\n", elapsed_secs);

    showDisparityMap(disparityMap, disparityRange);

    return 0;
}