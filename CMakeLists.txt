cmake_minimum_required(VERSION 3.16)
project(doc_cam)
set(CMAKE_CXX_STANDARD 17)
find_package( OpenCV REQUIRED )

add_subdirectory(doc-cam-core)