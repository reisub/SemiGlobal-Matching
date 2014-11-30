#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

int main( int argc, char** argv ) {
    if (argc != 3) {
     std::cerr << "Usage: " << argv[0] << " <first image> <second image>" << std::endl;
     return -1;
    }

    cv::Mat first_image;
    cv::Mat second_image;
    first_image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);
    second_image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);

    if(!first_image.data || !second_image.data) {
        std::cerr <<  "Could not open or find one of the images!" << std::endl;
        return -1;
    }

    return 0;
}