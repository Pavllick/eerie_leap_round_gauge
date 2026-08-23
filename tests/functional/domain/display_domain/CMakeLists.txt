cmake_minimum_required(VERSION 3.20.0)

set(RT_GAUGE_TEST_SIM_FLASH ON)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../rt_gauge_test.cmake)

# Use Zephyr's build system
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(functional_display_domain)

include_directories(${APP_SRC_DIR})

set(APP_SRC_CONFIGURATION_DIR ${APP_SRC_DIR}/configuration)
file(GLOB_RECURSE APP_SRC_CONFIGURATION
  ${APP_SRC_CONFIGURATION_DIR}/*.c
  ${APP_SRC_CONFIGURATION_DIR}/*.cpp
)

# Only the configuration half of the domain: DisplayService talks to the display
# driver, which the headless suites do not bind.
set(APP_SRC_DISPLAY_DOMAIN_DIR ${APP_SRC_DIR}/domain/display_domain/configuration)
file(GLOB_RECURSE APP_SRC_DISPLAY_DOMAIN
  ${APP_SRC_DISPLAY_DOMAIN_DIR}/*.c
  ${APP_SRC_DISPLAY_DOMAIN_DIR}/*.cpp
)

file(GLOB_RECURSE TEST_SOURCES src/*.c src/*.cpp)

# Add all source files to the app target
target_sources(app PRIVATE
  ${APP_SRC_CONFIGURATION}
  ${APP_SRC_DISPLAY_DOMAIN}
  ${TEST_SOURCES}
)
