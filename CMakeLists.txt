cmake_minimum_required(VERSION 3.16)
project(doc_cam)
set(CMAKE_CXX_STANDARD 17)
find_package( OpenCV REQUIRED )

add_executable(doc_cam main.cpp img_source.cpp)
target_include_directories(doc_cam SYSTEM PRIVATE /usr/include/opencv4)
target_link_libraries(doc_cam ${OpenCV_LIBS})
