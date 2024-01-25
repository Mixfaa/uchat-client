cmake_minimum_required(VERSION 3.7.0)
project(json_c_test VERSION 0.1.0 LANGUAGES C)

set(OpenGL_GL_PREFERENCE "LEGACY")

file (GLOB source
    "inc/*.h"
    "src/*.c"    
)

file (GLOB socket_uchat
    "inc/chat-socket/*.h"
    "src/chat-socket/*.c"
)

add_executable(${PROJECT_NAME} ${socket_uchat} ${source})
target_include_directories(${PROJECT_NAME} PRIVATE "inc")
add_definitions(-DCIMGUI_DEFINE_ENUMS_AND_STRUCTS -DCIMGUI_USE_GLFW -DCIMGUI_USE_OPENGL3)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL ${CMAKE_SOURCE_DIR}/libs/libcimgui.so ${CMAKE_SOURCE_DIR}/libs/libjson-c.so)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
