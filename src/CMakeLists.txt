cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX_STANDARD 17)

project(fisher VERSION 0.1.0)

set(OpenCV_DIR "../../opencv-mobile/x64")
set(ncnn_INSTALL_DIR "../../ncnn/x64")
set(ncnn_DIR "${ncnn_INSTALL_DIR}/lib/cmake/ncnn")
set(ncnn_INCLUDE_DIR "${ncnn_INSTALL_DIR}/include")

set(OpenCV_STATIC ON)

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

set(
    FISHER_SRCS

    main.cpp
    fishing.cpp
    fishnet/nanodet.cpp
    rodnet/rodnet.cpp
    utils/screenshot.cpp
)

add_executable(fisher ${FISHER_SRCS})

SET_TARGET_PROPERTIES(fisher PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")

target_link_libraries(
    fisher
    ncnn
    ${OpenCV_LIBS}
)