cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# add test directory as define
add_definitions("-DTENGU_TEST_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"")

set(SOURCE_FILES
    test_stb_vorbis.cpp)
	
if (MSVC)
    add_definitions(-D_SDL_main_h)
endif()

add_executable(test_stb_vorbis ${SOURCE_FILES})
target_link_libraries(test_stb_vorbis framework_core framework_audio SDL)

set_target_properties(test_stb_vorbis PROPERTIES
    FOLDER tests
)
