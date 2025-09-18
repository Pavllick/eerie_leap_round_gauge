cmake_minimum_required(VERSION 3.20)

# Apply patches before build
function(apply_patches patch_dir target_dir)
    file(GLOB_RECURSE files RELATIVE ${CMAKE_SOURCE_DIR} "${patch_dir}/*.patch")
    foreach(file ${files})
        execute_process(
            COMMAND git apply --reverse --check ${CMAKE_CURRENT_SOURCE_DIR}/${file} --unsafe-paths
            WORKING_DIRECTORY ${target_dir}
            RESULT_VARIABLE patch_already_applied
            OUTPUT_QUIET
            ERROR_QUIET
        )
        if(patch_already_applied EQUAL 0)
            message("Patch already applied: ${file}")
        else()
            message("Applying patch: ${file}")
            execute_process(
                COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/${file} --unsafe-paths
                WORKING_DIRECTORY ${target_dir}
                RESULT_VARIABLE patch_apply_result
            )
            if(NOT patch_apply_result EQUAL 0)
                message(FATAL_ERROR "Failed to apply patch: ${file}")
            endif()
        endif()
    endforeach()
endfunction()

apply_patches("patches/zephyr" $ENV{ZEPHYR_BASE})
apply_patches("patches/modules/lvgl" $ENV{ZEPHYR_BASE}/../modules/lib/gui/lvgl)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(eerie_leap_round_gauge)

# Enable compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++23 globally (Zephyr-compatible way)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect all source files
file(GLOB_RECURSE APP_SOURCES
  src/*.cpp
  src/*.c
)

# Add sources to the app target
target_sources(app PRIVATE ${APP_SOURCES})

# Include headers
target_include_directories(app PRIVATE src)
