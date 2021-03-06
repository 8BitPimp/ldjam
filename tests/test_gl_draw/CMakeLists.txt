cmake_minimum_required(VERSION 3.4)

find_package(OpenGL)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# add test directory as define
add_definitions("-DTENGU_TEST_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"")

set(SOURCE_FILES
    test_gl_draw.cpp)

if (MSVC)
    add_definitions(-D_SDL_main_h)
endif()

set(LIBRARIES
    framework_core
    framework_draw
    framework_gl_draw 
    SDL
    glee
    ${OPENGL_LIBRARIES})
    
add_executable(test_gl_draw ${SOURCE_FILES})
target_link_libraries(test_gl_draw PUBLIC ${LIBRARIES})

set_target_properties(test_gl_draw PROPERTIES
    FOLDER tests
)
