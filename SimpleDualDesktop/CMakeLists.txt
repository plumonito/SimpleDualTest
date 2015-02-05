##################################
# Desktop application
##################################

include_directories(${MY_DEPENDENCY_DIRS})

##################################
# Sources
if(WIN32)
    #Windows libraries
    set(ExtraLib Winmm)
endif()

#Add all files
file(GLOB_RECURSE sources_cpp src/*.cpp)
file(GLOB_RECURSE sources_h src/*.h)

##################################
# Target

add_definitions(-DGLEW_STATIC)
include_directories ("${PROJECT_SOURCE_DIR}/SimpleDualApp/src")

add_executable(SimpleDualDesktop ${sources_cpp} ${sources_h})
target_link_libraries (SimpleDualDesktop native_app SimpleDualApp ${ExtraLib})


#INSTALL_TARGET_WITH_DEPS(SimpleDualDesktop ${3RD_PARTY_DIR}/bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/modules/SimpleDualApp/assets DESTINATION .)

add_custom_command(TARGET SimpleDualDesktop POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/modules/SimpleDualApp/assets $<TARGET_FILE_DIR:SimpleDualDesktop>/assets)
	