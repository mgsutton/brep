cmake_minimum_required(VERSION 3.1...3.13)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
	cmake_policy(VERSION 3.13)
endif()

project(BRep VERSION 1.0
	         DESCRIPTION "A simple C++ static geometry brep structure"
	         LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Packages
find_package(Eigen3 REQUIRED)

if(EIGEN3_FOUND)
	message("Eigen3 found")
else()
    message(FATAL_ERROR "Could not find the eigen libraries...")
endif()

find_package(glfw3 CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)



add_subdirectory(src)
add_subdirectory(apps)
