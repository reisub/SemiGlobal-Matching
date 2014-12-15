#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <ctime>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "gaussian.h"

#define BLUR_RADIUS 3
#define PATH_COUNT 8
#define MAX_SHORT 65535
#define SMALL_PENALTY 3
#define LARGE_PENALTY 20
#define DEBUG false

struct path {
    short rowDiff;
    short colDiff;
    short index;
};

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

void initializePaths(std::vector<path> &paths, unsigned short pathCount) {
    for (unsigned short i = 0; i < pathCount; ++i) {
        paths.push_back(path());
    }

    if(paths.size() >= 4) {
        paths[0].rowDiff = -1;
        paths[0].colDiff = 0;
        paths[0].index = 0;

        paths[1].rowDiff = 0;
        paths[1].colDiff = 1;
        paths[1].index = 1;

        paths[2].rowDiff = 1;
        paths[2].colDiff = 0;
        paths[2].index = 2;

        paths[3].rowDiff = 0;
        paths[3].colDiff = -1;
        paths[3].index = 3;
    }

    if(paths.size() >= 8) {
        paths[4].rowDiff = -1;
        paths[4].colDiff = 1;
        paths[4].index = 4;

        paths[5].rowDiff = 1;
        paths[5].colDiff = 1;
        paths[5].index = 5;

        paths[6].rowDiff = 1;
        paths[6].colDiff = -1;
        paths[6].index = 6;

        paths[7].rowDiff = -1;
        paths[7].colDiff = -1;
        paths[7].index = 7;
    }

    if(paths.size() >= 16) {
        paths[8].rowDiff = -2;
        paths[8].colDiff = 1;
        paths[8].index = 8;

        paths[9].rowDiff = -2;
        paths[9].colDiff = -1;
        paths[9].index = 9;

        paths[10].rowDiff = 2;
        paths[10].colDiff = 1;
        paths[10].index = 10;

        paths[11].rowDiff = 2;
        paths[11].colDiff = -1;
        paths[11].index = 11;

        paths[12].rowDiff = 1;
        paths[12].colDiff = -2;
        paths[12].index = 12;

        paths[13].rowDiff = -1;
        paths[13].colDiff = -2;
        paths[13].index = 13;

        paths[14].rowDiff = 1;
        paths[14].colDiff = 2;
        paths[14].index = 14;

        paths[15].rowDiff = -1;
        paths[15].colDiff = 2;
        paths[15].index = 15;
    }
}

unsigned short aggregateCost(int row, int col, int d, path &p, int rows, int cols, int disparityRange, unsigned short ***C, unsigned short ****A, unsigned short ***S, unsigned int iter) {
    unsigned short aggregatedCost = 0;
    aggregatedCost += C[row][col][d];

    if(DEBUG) {
        for (unsigned int i = 0; i < iter; ++i) {
            printf(" ");
        }
        printf("{P%d}[%d][%d](d%d)\n", p.index, row, col, d);
    }

    if (A[p.index][row][col][d] != MAX_SHORT) {
        if(DEBUG) {
            for (unsigned int i = 0; i < iter; ++i) {
                printf(" ");
            }
            printf("{P%d}[%d][%d](d%d)-> %d<CACHED>\n", p.index, row, col, d, A[p.index][row][col][d]);
        }
        return A[p.index][row][col][d];
    }

    if (row + p.rowDiff < 0 || row + p.rowDiff >= rows || col + p.colDiff < 0 || col + p.colDiff >= cols) {
        // border
        A[p.index][row][col][d] = aggregatedCost;
        if(DEBUG) {
            for (unsigned int i = 0; i < iter; ++i) {
                printf(" ");
            }
            printf("{P%d}[%d][%d](d%d)-> %d <BORDER>\n", p.index, row, col, d, A[p.index][row][col][d]);
        }
        return A[p.index][row][col][d];

        return aggregatedCost;
    }

    unsigned short minPrev, minPrevOther, prev, prevPlus, prevMinus;
    minPrev = minPrevOther = prevPlus = prevMinus = MAX_SHORT;

    for (int disp = 0; disp < disparityRange; ++disp) {
        unsigned short tmp = aggregateCost(row + p.rowDiff, col + p.colDiff, disp, p, rows, cols, disparityRange, C, A, S, ++iter);
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

    A[p.index][row][col][d] = aggregatedCost;

    if(DEBUG) {
        for (unsigned int i = 0; i < iter; ++i) {
            printf(" ");
        }
        printf("{P%d}[%d][%d](d%d)-> %d<CALCULATED>\n", p.index, row, col, d, A[p.index][row][col][d]);
    }

    return aggregatedCost;
}

void aggregateCosts(int rows, int cols, int disparityRange, std::vector<path> &paths, unsigned short ***C, unsigned short ****A, unsigned short ***S) {
    #pragma omp parallel for
    for (unsigned long p = 0; p < paths.size(); ++p) {
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                for (int d = 0; d < disparityRange; ++d) {
                    S[row][col][d] += aggregateCost(row, col, d, paths[p], rows, cols, disparityRange, C, A, S, 0);
                }
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

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <left image> <right image> <disparity range>" << std::endl;
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

    unsigned int disparityRange = atoi(argv[3]);
    unsigned short ***C; // pixel cost array W x H x D
    unsigned short ***S; // aggregated cost array W x H x D
    unsigned short ****A; // path cost array P x W x H x D
    std::vector<path> paths;

    initializePaths(paths, PATH_COUNT);

    /*
    * TODO
    * variable LARGE_PENALTY
    */

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

    A = new unsigned short ***[paths.size()];
    for (unsigned long p = 0; p < paths.size(); ++p) {
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

    std::cout << "Smoothing images..." << std::endl;
    grayscaleGaussianBlur(firstImage, firstImage, BLUR_RADIUS);
    grayscaleGaussianBlur(secondImage, secondImage, BLUR_RADIUS);

    std::cout << "Calculating pixel cost for the image..." << std::endl;
    calculatePixelCost(firstImage, secondImage, disparityRange, C);
    if(DEBUG) {
        printArray(C, firstImage.rows, firstImage.cols, disparityRange);
    }
    std::cout << "Aggregating costs..." << std::endl;
    aggregateCosts(firstImage.rows, firstImage.cols, disparityRange, paths, C, A, S);

    cv::Mat disparityMap = cv::Mat(cv::Size(firstImage.cols, firstImage.rows), CV_8UC1, cv::Scalar::all(0));

    std::cout << "Computing disparity..." << std::endl;
    computeDisparity(S, firstImage.rows, firstImage.cols, disparityRange, disparityMap);

    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    printf("Done in %.2lf seconds.\n", elapsed_secs);

    showDisparityMap(disparityMap, disparityRange);

    return 0;
}