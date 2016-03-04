cmake_minimum_required(VERSION 3.1)

project(plot-test CXX)
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/../install")

# GLFW
if(MSVC)
    set(GLFW_DIR "${CMAKE_SOURCE_DIR}/lib/glfw-3.1.2.bin.WIN64")
    include_directories("${GLFW_DIR}/include")
    if(MSVC11)
        set(GLFW_LIB_SUBDIR "lib-vc2012")
    elseif(MSVC12)
        set(GLFW_LIB_SUBDIR "lib-vc2013")
    elseif(MSVC14)
        set(GLFW_LIB_SUBDIR "lib-vc2015")
    endif()
    find_library(GLFW_LIBRARY glfw3 "${GLFW_DIR}/${GLFW_LIB_SUBDIR}")
    link_libraries("${GLFW_LIBRARY}")
endif()

# GLEW
if(MSVC)
    set(GLEW_DIR "${CMAKE_SOURCE_DIR}/lib/glew-1.13.0")
    include_directories("${GLEW_DIR}/include")
    find_library(GLEW_LIBRARY glew32 "${GLEW_DIR}/lib/Release/x64")
	link_libraries("${GLEW_LIBRARY}")
endif()

# GLM
set(GLM_DIR "${CMAKE_SOURCE_DIR}/lib/glm")
include_directories("${GLM_DIR}")

# TCLAP
set(TCLAP_DIR "${CMAKE_SOURCE_DIR}/lib/tclap-1.2.1")
include_directories("${TCLAP_DIR}/include")

# Link Opengl32.lib with MSVC
if(MSVC)
    link_libraries(opengl32)
endif()

# Build plotter
add_subdirectory(src)
