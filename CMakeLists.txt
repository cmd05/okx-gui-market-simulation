cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0076 NEW)

include(FetchContent)

project(client_trader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# boost library
find_package(Boost REQUIRED COMPONENTS system thread)

# openssl library
find_package(OpenSSL REQUIRED)

# nlohmann-json library
# https://json.nlohmann.me/integration/cmake/#fetchcontent
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(json)

# websocketpp library
# https://github.com/zaphoyd/websocketpp/issues/990
FetchContent_Declare(websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG master
)

FetchContent_MakeAvailable(websocketpp)

### GLFW
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.4
)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

### Glad
if(NOT TARGET glad)
    add_subdirectory(./includes/glad)
endif()

### ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    # use "docking" branch
    GIT_TAG 126d004f9e1eef062bf4b044b3b2faaf58d48c51 # ver 1.91.9 (docking)
)

FetchContent_MakeAvailable(imgui)

add_library(imgui)

set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR})
target_include_directories(imgui PUBLIC 
    ${IMGUI_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}/backends
    ${IMGUI_INCLUDE_DIR}/misc/cpp
)

target_sources(imgui
    PUBLIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp

    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(imgui PRIVATE glfw)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -pg -g1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

add_executable(client_trader)

target_sources(client_trader
    PRIVATE
    src/client_main.cpp
    src/websocket/websocket.cpp
)

target_include_directories(client_trader 
    PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${websocketpp_SOURCE_DIR}
)

target_include_directories(client_trader
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(client_trader 
    PRIVATE 
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
    Boost::thread
    nlohmann_json::nlohmann_json
    glfw
    imgui
    glad
)

set_target_properties(client_trader PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")