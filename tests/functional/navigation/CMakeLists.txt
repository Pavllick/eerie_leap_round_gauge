cmake_minimum_required(VERSION 3.20.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../rt_gauge_test.cmake)

# Use Zephyr's build system
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(functional_navigation)

include_directories(${APP_SRC_DIR})

file(GLOB_RECURSE TEST_SOURCES src/*.c src/*.cpp)

# NavigationService reaches no further than the UI event bus, which needs LvglLock
# for its dispatch guards. Neither pulls in LVGL or views, so this suite stays
# runnable on every platform.
target_sources(app PRIVATE
  ${APP_SRC_DIR}/domain/ui_domain/lvgl_lock.cpp
  ${APP_SRC_DIR}/domain/ui_domain/services/navigation_service.cpp
  ${TEST_SOURCES}
)
