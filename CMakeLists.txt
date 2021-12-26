# CMakeList.txt : CMake project for ShaderReflection, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

cmake_policy(SET CMP0054 NEW)

project ("ShaderReflection")

# Detect Linux
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (GGP)
    message("GGP platform")
elseif (LINUX)
    message("LINUX platform")
elseif (WIN32)
    message("WIN32 platform")
endif()

# ------------------------------------------------------------------------------
# Configure C++ flags
# ------------------------------------------------------------------------------
if(MSVC)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Zc:__cplusplus /std:c++17")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

set(CMAKE_CXX_STANDARD 17)

set(SR_DIR    ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR    ${SR_DIR}/src)
set(SR_THIRD_PARTY_DIR ${SR_DIR}/third_party)
set(DXC_DIR    ${SR_THIRD_PARTY_DIR}/dxc)
set(TARGET_NAME ShaderReflection)

find_path(DXC_INCLUDE_DIR dxcapi.h d3d12shader.h)
find_library(DXC_LIBRARY dxcompiler)

if(DXC_INCLUDE_DIR AND DXC_LIBRARY)
    # you may need that if further action in your CMakeLists.txt depends
    # on detecting your library
    message("I found DXC!!!")
    #set(SIFTGPU_FOUND TRUE)

    # you may need that if you want to conditionally compile some parts
    # of your code depending on library availability
    #add_definitions(-DHAVE_LIBSIFTGPU=1)
else()
    message("Oh noooo! I found nothing!!!")
    set(DXC_INCLUDE_DIR ${DXC_DIR}/inc)
    set(DXC_LIBRARY ${DXC_DIR}/lib/x64/dxcompiler.lib)
endif()

include_directories(${DXC_INCLUDE_DIR})
set(ALL_LIBRARIES ${ALL_LIBRARIES} ${DXC_LIBRARY})

message("DXC_INCLUDE_DIR", ${DXC_INCLUDE_DIR})
message("ALL_LIBRARIES", ${DXC_LIBRARY})

#add_subdirectory(${DXC_DIR})

# Add source to this project's executable.
add_executable (${TARGET_NAME} "src/ShaderReflection.cpp" "include/ShaderReflection.h")

target_include_directories(
            ${TARGET_NAME}
            PUBLIC ${SR_DIR}/include
                    ${DXC_INCLUDE_DIR}
        )

target_link_libraries(${TARGET_NAME} ${ALL_LIBRARIES})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
