#ifndef _IMG_SOURCE_H_
#define _IMG_SOURCE_H_

#include <opencv2/opencv.hpp>
#include <string>

struct ImageSource {
  virtual cv::Mat get_image() = 0;
};

struct FileImageSource: public ImageSource {
  std::string filename; 
  cv::Mat get_image() override;
};
#endif
