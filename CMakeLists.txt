cmake_minimum_required(VERSION 3.28)
project(kintematic_example LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG master
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)


file(GLOB_RECURSE SRC_SOURCES CONFIGURE_DEPENDS "source/*.cpp")
file(GLOB_RECURSE SRC_HEADERS CONFIGURE_DEPENDS "source/*.hpp")
set (SRC_INCLUDE_DIRS "")
foreach (_headerFile ${SRC_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND SRC_INCLUDE_DIRS ${_dir})
endforeach()
list (REMOVE_DUPLICATES SRC_INCLUDE_DIRS)

add_executable(main ${SRC_SOURCES})
target_link_libraries(main PRIVATE SFML::Graphics)

target_compile_features(main PRIVATE cxx_std_23)
# target_compile_options(main PRIVATE -Wall -Wextra -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wfloat-equal -Wunreachable-code -Werror)
target_compile_options(main PRIVATE -g)
