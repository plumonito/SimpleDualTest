##################################
# native_env application
##################################

##################################
# Dependencies

add_definitions(-DGLEW_STATIC)

##################################
# Sources

#Add all files
file(GLOB_RECURSE sources_cpp src/*.cpp)
file(GLOB_RECURSE sources_h src/*.h)

include_directories(src)
include_directories ("${PROJECT_SOURCE_DIR}/modules/native_env_core/src")
include_directories ("${PROJECT_SOURCE_DIR}/modules/native_env_core/win32")

##################################
# Target

add_library(native_env ${sources_cpp} ${sources_h})
target_link_libraries(native_env ${LIBS})

