#include "img_source.h"

cv::Mat FileImageSource::get_image() {
    using namespace cv;
    Mat image = imread(filename, IMREAD_LOAD_GDAL|IMREAD_COLOR );
    return std::move(image);
}
