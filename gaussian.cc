#include "gaussian.h"

#define PI 3.141592654

// simple wrapper for accessing single-channel matrix elements
inline uchar getMatElement(cv::Mat &matrix, int i, int j) {
    assert(matrix.type() == CV_8UC1);
    if (i < 0 || i >= matrix.rows || j < 0 || j >= matrix.cols) {
        return 0;
    }
    else {
        return matrix.at<uchar>(i, j);
    }
}

void grayscaleGaussianBlur(cv::Mat &source, cv::Mat &destination, int sizeX, int sizeY) {
    if(sizeY == -1) {
        sizeY = sizeX;
    }

    // assert that the kernel sizes are odd and positive
    assert(sizeX % 2 == 1);
    assert(sizeY % 2 == 1);

    // assert that the image matrices are grayscale
    assert(source.type() == CV_8UC1);
    assert(destination.type() == CV_8UC1);

    // assert that the image matrices are of the same size
    assert(source.size() == destination.size());

    int radiusX = (sizeX-1)/2;
    int radiusY = (sizeY-1)/2;

    double *kernelX = new double[sizeX];
    double *kernelY = new double[sizeY];

    // sigma determines the "spread" of the "Gaussian bell"
    double sigmaX = radiusX/2.0;
    double sigmaY = radiusY/2.0;

    double x, y, sum = 0;

    // calculate kernels and normalize them
    for (int i = 0; i < sizeX; ++i) {
        x = abs(radiusX - i);
        kernelX[i] = (1/sqrt(2*PI*sigmaX*sigmaX))*exp(-(x*x)/(2*sigmaX*sigmaX));
        sum += kernelX[i];
    }
    for (int i = 0; i < sizeX; ++i) {
        kernelX[i] /= sum;
    }

    // if the sizes are equal, the kernels also are
    if(sizeX == sizeY) {
        delete [] kernelY;
        kernelY = kernelX;
    }
    else {
        sum = 0;
        for (int i = 0; i < sizeY; ++i) {
            y = abs(radiusY - i);
            kernelY[i] = (1/sqrt(2*PI*sigmaY*sigmaY))*exp(-(y*y)/(2*sigmaY*sigmaY));
            sum += kernelY[i];
        }
        for( int i = 0; i < sizeY; ++i ) {
            kernelY[i] /= sum;
        }
    }

    cv::Mat tmp = destination.clone();

    // convolve image with kernelX
    double valueX, valueY;
    for (int i = 0; i < source.rows; ++i) {
        for (int j = 0; j < source.cols; ++j) {
            valueX = 0;
            for (int k = 0; k < sizeX; ++k) {
                valueX += kernelX[k] * getMatElement(source, i, j + k - radiusX);
            }
            tmp.at<uchar>(i, j) = (uchar)(valueX);
        }
    }

    // convolve image with kernelY
    for (int i = 0; i < tmp.rows; ++i) {
        for (int j = 0; j < tmp.cols; ++j) {
            valueY = 0;
            for (int k = 0; k < sizeY; ++k) {
                valueY += kernelY[k] * getMatElement(tmp, i + k - radiusY, j);
            }
            destination.at<uchar>(i, j) = (uchar)(valueY);
        }
    }

    // dealocate space
    delete [] kernelX;
    if(sizeY != sizeX) {
        delete [] kernelY;
    }
}