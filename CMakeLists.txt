cmake_minimum_required(VERSION 3.14)

set(project_name datastructures)
set(project_test ${project_name}_test)
project(${project_name})

MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list include/*.h)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -g")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -fsanitize=address")
set(CMAKE_VERBOSE_MAKEFILE ON)

HEADER_DIRECTORIES(header_directories)
include_directories(${header_directories})

file(GLOB_RECURSE source_files src/*.c)

add_library(${project_name} ${source_files})

if (WIN32)
find_library(pthread NAME pthread)
target_link_libraries(${project_name} pthread)
target_link_libraries(${project_name} -static)
endif()

enable_testing()

file(GLOB_RECURSE test_filepaths tests/*.c)
foreach(test_filepath ${test_filepaths})
    get_filename_component(test_file ${test_filepath} NAME_WE)
    add_executable(${test_file} ${test_filepath})
    target_link_libraries(${test_file} ${project_name})
    add_test(
        NAME ${test_file}
        COMMAND $<TARGET_FILE:${test_file}>
    )
endforeach()