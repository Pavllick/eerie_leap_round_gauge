cmake_minimum_required(VERSION 3.20.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../rt_gauge_test.cmake)

# Use Zephyr's build system
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(functional_event_bus)

include_directories(${APP_SRC_DIR})

set(APP_SRC_EVENT_BUS_DIR ${APP_SRC_DIR}/event_bus)
file(GLOB_RECURSE APP_SRC_EVENT_BUS
  ${APP_SRC_EVENT_BUS_DIR}/*.c
  ${APP_SRC_EVENT_BUS_DIR}/*.cpp
)

file(GLOB_RECURSE TEST_SOURCES src/*.c src/*.cpp)

# Add all source files to the app target
target_sources(app PRIVATE
  ${APP_SRC_EVENT_BUS}
  ${TEST_SOURCES}
)
