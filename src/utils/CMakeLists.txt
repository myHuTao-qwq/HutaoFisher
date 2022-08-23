set(OpenCV_DIR "C:/Program Files/opencv/build")

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_library(screenshot screenshot.cpp)

target_link_libraries(screenshot ${OpenCV_LIBS})

