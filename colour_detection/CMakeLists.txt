cmake_minimum_required(VERSION 2.8)
project( detect_color )
find_package( OpenCV REQUIRED )
add_executable(detect_color detect_color.cpp )
target_link_libraries( detect_color ${OpenCV_LIBS} )
