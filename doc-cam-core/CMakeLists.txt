cmake_minimum_required(VERSION 3.16)
project(doc_cam_core)

add_executable(doc_cam_core main.cpp img_source.cpp)
target_include_directories(doc_cam_core SYSTEM PRIVATE /usr/include/opencv4)
target_link_libraries(doc_cam_core ${OpenCV_LIBS})
