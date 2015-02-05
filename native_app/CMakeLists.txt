##################################
# native_app library
##################################

##################################
# Dependencies

add_definitions(-DGLEW_STATIC)

##################################
# Sources

#Add all files
file(GLOB_RECURSE sources_h src/*.h)
file(GLOB_RECURSE sources_win32_cpp win32/*.cpp)
file(GLOB_RECURSE sources_win32_c win32/*.c)
file(GLOB_RECURSE sources_win32_h win32/*.h)

include_directories(src)

include_directories(win32)
include_directories ("${PROJECT_SOURCE_DIR}/modules/native_env_core/src")
include_directories ("${PROJECT_SOURCE_DIR}/modules/native_env_core/win32")
include_directories ("${PROJECT_SOURCE_DIR}/modules/native_env/src")

##################################
# Target

add_library(native_app ${sources_h} ${sources_win32_cpp} ${sources_win32_c} ${sources_win32_h})
target_link_libraries(native_env ${LIBS})

