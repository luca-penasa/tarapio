cmake_minimum_required(VERSION 2.8)
project(tarapio)

        if (NOT CMAKE_BUILD_TYPE)
                message(STATUS "No build type selected, default to Release")
                set(CMAKE_BUILD_TYPE "Release")
        endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -std=c++11 -fopenmp -fPIC")

find_package( OpenCV REQUIRED )


file(GLOB SOURCES *.cpp)
file(GLOB HEADERS *.h)
set(files "${SOURCES}" "${HEADERS}")


add_executable(${PROJECT_NAME} ${files})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)


target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})



