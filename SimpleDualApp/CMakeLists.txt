##################################
# SimpleDualApp library
##################################

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${Eigen_INCLUDE_DIR})
include_directories(${OpenGL_INCLUDE_DIRS})
add_definitions(-DGLEW_STATIC)

##################################
# Sources

#Add all files
file(GLOB_RECURSE my_sources_cpp src/*.cpp)
file(GLOB_RECURSE my_sources_h src/*.h)

#Remove android files
#file(GLOB_RECURSE nvslam_android_cpp src/system/android/*.cpp)
#list(REMOVE_ITEM nvslam_sources_cpp ${nvslam_android_cpp})
#include_directories ("${PROJECT_SOURCE_DIR}/modules/native_camera/src")

##################################
# Target
include_directories ("${PROJECT_SOURCE_DIR}/modules/nvslam/src")

add_library(SimpleDualApp ${my_sources_cpp} ${my_sources_h})
target_link_libraries(SimpleDualApp native_env native_env_core ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})