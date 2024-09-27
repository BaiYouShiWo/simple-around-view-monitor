cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXXSTANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
project(bev)

include_directories(./include)
#add_subdirectory(include)
#add_subdirectory(src)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(bev ${SRC_FILES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavutil)
include_directories(${FFMPEG_INCLUDE_DIRS})

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

target_link_libraries( ${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES} ${OpenCV_LIBS} avformat avcodec avutil swscale pthread stdc++fs)




