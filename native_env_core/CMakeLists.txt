##################################
# native_env_core application
##################################

##################################
# Dependencies

add_definitions(-DGLEW_STATIC)

##################################
# Sources

#Add all files
file(GLOB_RECURSE sources_cpp src/*.cpp)
file(GLOB_RECURSE sources_h src/*.h)
file(GLOB_RECURSE sources_win32_cpp win32/*.cpp)
file(GLOB_RECURSE sources_win32_c win32/*.c)
file(GLOB_RECURSE sources_win32_h win32/*.h)

include_directories(src)
include_directories(win32)

##################################
# Target

add_library(native_env_core ${sources_cpp} ${sources_h} ${sources_win32_cpp} ${sources_win32_c} ${sources_win32_h})
target_link_libraries(native_env_core ${LIBS})

