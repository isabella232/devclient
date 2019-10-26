cmake_minimum_required(VERSION 3.15)
project(devclient)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(contrib/fmt-6.0.0)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)
pkg_check_modules(GIOMM giomm-2.4)
pkg_check_modules(LIBFTDI libftdipp1)

link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GIOMM_INCLUDE_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})
include_directories(${LIBFTDI_INCLUDE_DIRS})
include_directories(include)

add_executable(devclient
        src/main.cc
        src/uart.cc
        src/jtag.cc
        src/device.cc
        src/log.cc
        src/deviceselect.cc
        src/mainwindow.cc)

target_link_libraries(devclient ${GIOMM_LIBRARIES} ${GTKMM_LIBRARIES} ${LIBFTDI_LIBRARIES} fmt)
target_link_libraries(devclient pthread)
