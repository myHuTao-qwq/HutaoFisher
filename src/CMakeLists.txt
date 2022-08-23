cmake_minimum_required(VERSION 3.0.0)

project(fisher VERSION 0.1.0)

add_subdirectory(utils)
add_subdirectory(fishnet)
add_subdirectory(rodnet)

include(CTest)
enable_testing()

set(OpenCV_DIR "C:/Program Files/opencv/build")

find_package(OpenCV REQUIRED)

include_directories()

# add_executable(fisher main.cpp getRodData.cpp)
add_executable(fisher main.cpp fishing.cpp)

target_link_libraries(fisher PUBLIC
    screenshot
    nanodet
    rodnet
)

target_include_directories(fisher PUBLIC
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


