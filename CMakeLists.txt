##################################
# SimpleDualTest project
##################################
cmake_minimum_required(VERSION 2.8)

project(SimpleDualTest CXX C)
# The version number.
set (SimpleDualTest_VERSION_MAJOR 1)
set (SimpleDualTest_VERSION_MINOR 0)


##################################
# Dependencies

#default location of pre-built dependencies
#OpenCV
find_package( OpenCV REQUIRED core calib3d highgui imgproc features2d video) 

#Eigen
find_path(Eigen_INCLUDE_DIR Eigen/Eigen)
include_directories(${Eigen_INCLUDE_DIR})

#Threads
find_package(Threads REQUIRED) #Result in ${CMAKE_THREAD_LIBS_INIT} 

#OpenGL
find_package(OpenGL REQUIRED)

#OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

#native libs
set(FRAMEWORK_DIRS "${PROJECT_SOURCE_DIR}/native_env_core/src" "${PROJECT_SOURCE_DIR}/native_env_core/win32" "${PROJECT_SOURCE_DIR}/native_env/src" "${PROJECT_SOURCE_DIR}/native_app/src" "${PROJECT_SOURCE_DIR}/native_app/win32")
include_directories(${FRAMEWORK_DIRS})

#General compiler flags
add_definitions(-DDEBUG_MODE) #This shows all messages from LOG()
add_definitions(-Wall) #Enable all warnings always
add_definitions(-DARCH_X86) #Not android

if(NOT MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE Release)
        message(STATUS "No build type specified, assuming release.")
    endif()

    set(CMAKE_ECLIPSE_MAKE_ARGUMENTS -j)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -mtune=corei7 -march=corei7")
    #add_definitions(-D__GXX_EXPERIMENTAL_CXX0X__=) #-std=gnu++0x is the same, but needed here for Eclipse's indexer
else()
    #glew
    #find_package(GLEW REQUIRED)

    #pthread
    find_path(pthread_INCLUDE_DIR pthread.h)
    find_library(pthread_LIBRARY pthreadVC2)
	include_directories(${pthread_INCLUDE_DIR})
	
    #set(NVSLAM_DEPENDENCY_DIRS ${NVSLAM_DEPENDENCY_DIRS} ${pthread_INCLUDE_DIR})
    #set(NVSLAM_DEPENDENCY_LIBS ${NVSLAM_DEPENDENCY_LIBS} ${pthread_LIBRARY})
    
    add_definitions(-DWIN32 -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX -DWIN32_LEAN_AND_MEAN)    
endif()


#macro for installing executable binary and dependencies
macro(INSTALL_TARGET_WITH_DEPS TARGET_NAME BIN_DIR)
    get_target_property(${TARGET_NAME}_LOCATION ${TARGET_NAME} LOCATION_RELEASE)
    get_filename_component(${TARGET_NAME}_LOCATION ${${TARGET_NAME}_LOCATION} NAME)
    set(${TARGET_NAME}_APP ${CMAKE_INSTALL_PREFIX}/${${TARGET_NAME}_LOCATION})
    set(${TARGET_NAME}_DIR ${BIN_DIR})
    install(TARGETS ${TARGET_NAME} DESTINATION .)
    install(CODE "include(BundleUtilities)\nfixup_bundle(\"${${TARGET_NAME}_APP}\" \"\" \"${${TARGET_NAME}_DIR}\")" COMPONENT Runtime)
endmacro(INSTALL_TARGET_WITH_DEPS)

add_subdirectory(native_env_core) 
add_subdirectory(native_env) 
add_subdirectory(native_app) 
add_subdirectory(SimpleDualApp) 
add_subdirectory(SimpleDualDesktop) 
