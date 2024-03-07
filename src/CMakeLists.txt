cmake_minimum_required(VERSION 3.4.1...3.5)
set(CMAKE_CXX_STANDARD 17)

project(fisher VERSION 4.4.1)

option(RELEASE "change directories" OFF)

if(NOT RELEASE)
    set(OpenCV_DIR "../../opencv/build") # collecting data needs cv::imshow
    option(TEST "collect rod data" OFF)
else()
    set(OpenCV_DIR "../../opencv-mobile/x64")
endif()


set(ncnn_INSTALL_DIR "../../ncnn/x64")
set(ncnn_DIR "${ncnn_INSTALL_DIR}/lib/cmake/ncnn")
set(ncnn_INCLUDE_DIR "${ncnn_INSTALL_DIR}/include")

set(OpenCV_STATIC ON) # warning: this option doesn't work with a pre-build opencv (not mobile)

find_package(OpenCV REQUIRED)

find_package(ncnn REQUIRED)
if(NOT TARGET ncnn)
    message(WARNING "ncnn NOT FOUND!  Please set ncnn_DIR environment variable")
else()
    message("ncnn FOUND ")
endif()

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${ncnn_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(config.h.in config.h)

set(
    FISHER_SRCS

    main.cpp
    fishing.cpp
    nanodet.cpp
    rodnet.cpp
    screenshot.cpp
)

add_executable(fisher ${FISHER_SRCS})

SET_TARGET_PROPERTIES(fisher PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")

target_link_libraries(
    fisher
    ncnn
    dwmapi
    ${OpenCV_LIBS}
)