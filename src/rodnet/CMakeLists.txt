cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX_STANDARD 17)

message("rodnet")

set(LIB_DIR "C:/Users/LGA11/Coding/genshin_auto_fish_v2/deploy/lib")

set(ncnn_DIR "${LIB_DIR}/ncnn/lib/cmake/ncnn" CACHE PATH "${LIB_DIR}/ncnn/lib/cmake/ncnn" FORCE)

find_package(ncnn REQUIRED)
if(NOT TARGET ncnn)
    message(WARNING "ncnn NOT FOUND!  Please set ncnn_DIR environment variable")
else()
    message("ncnn FOUND ")
endif()

add_library(rodnet rodnet.cpp)

target_link_libraries(rodnet ncnn ${OpenCV_LIBS})

target_include_directories(rodnet PUBLIC
    # "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.19041.0\\winrt"
    ${LIB_DIR}/ncnn/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)