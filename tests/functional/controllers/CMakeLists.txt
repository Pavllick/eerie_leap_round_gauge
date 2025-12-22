cmake_minimum_required(VERSION 3.20.0)

set(EXTRA_ZEPHYR_MODULES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/eerie_leap_common")

# Use Zephyr's build system
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_guid_generator)

# Enable C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-DCMAKE_EXPORT_COMPILE_COMMANDS=1)

set(APP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../app/src)
include_directories(${APP_SRC_DIR})

set(APP_SRC_CONFIGURATION_DIR ${APP_SRC_DIR}/configuration)
file(GLOB_RECURSE APP_SRC_CONFIGURATION
  ${APP_SRC_CONFIGURATION_DIR}/*.c
  ${APP_SRC_CONFIGURATION_DIR}/*.cpp
)

set(APP_SRC_SYSTEM_DOMAIN_DIR ${APP_SRC_DIR}/domain/system_domain)
file(GLOB_RECURSE APP_SRC_SYSTEM_DOMAIN
  ${APP_SRC_SYSTEM_DOMAIN_DIR}/*.c
  ${APP_SRC_SYSTEM_DOMAIN_DIR}/*.cpp
)

set(APP_SRC_UI_DOMAIN_DIR ${APP_SRC_DIR}/domain/ui_domain)
file(GLOB_RECURSE APP_SRC_UI_DOMAIN
  ${APP_SRC_UI_DOMAIN_DIR}/*.c
  ${APP_SRC_UI_DOMAIN_DIR}/*.cpp
)

set(APP_SRC_SUBSYS_DT_DIR ${APP_SRC_DIR}/subsys/device_tree)
file(GLOB_RECURSE APP_SRC_SUBSYS_DT
  ${APP_SRC_SUBSYS_DT_DIR}/dt_fs.cpp
)

file(GLOB_RECURSE TEST_SOURCES src/*.c src/*.cpp)

# Add all source files to the app target
target_sources(app PRIVATE
  ${APP_SRC_CONFIGURATION}
  ${APP_SRC_SYSTEM_DOMAIN}
  ${APP_SRC_UI_DOMAIN}
  ${APP_SRC_SUBSYS_DT}
  ${TEST_SOURCES}
)
