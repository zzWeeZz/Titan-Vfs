cmake_minimum_required(VERSION 3.50)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)
# Project name and version
project(Titan-Vfs VERSION 1.0 LANGUAGES CXX)
option(BUILD_EXAMPLES "Build examples" OFF)
# Add the miniz-cpp library
add_subdirectory(vendor/miniz-cpp EXCLUDE_FROM_ALL)
add_subdirectory(examples)

# Fetch dependencies
find_package(EASTL QUIET)
if (NOT EASTL_FOUND)
    FetchContent_Declare(
            EASTL
            GIT_REPOSITORY https://github.com/zzWeeZz/EASTL.git
    )
    FetchContent_MakeAvailable(EASTL)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the include directory as a target
add_library(Titan-Vfs INTERFACE)
add_library(Titan::Vfs ALIAS Titan-Vfs)

target_link_libraries(Titan-Vfs INTERFACE EASTL)

target_include_directories(Titan-Vfs
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/miniz-cpp>
        $<BUILD_INTERFACE:${EASTL_SOURCE_DIR}/include>
)

target_compile_features(Titan-Vfs INTERFACE cxx_std_17)


if (BUILD_EXAMPLES)
add_subdirectory(examples)
endif()
