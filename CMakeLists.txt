cmake_minimum_required(VERSION 3.15)
project(devclient)
set(CMAKE_CXX_STANDARD 17)

set(OPENOCD_URL https://github.com/conclusiveeng/openocd.git)
set(DTC_URL git://git.kernel.org/pub/scm/utils/dtc/dtc.git)

add_subdirectory(contrib/fmt-6.0.0)
add_subdirectory(contrib/filesystem-1.2.10)

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
        src/i2c.cc
        src/gpio.cc
        src/device.cc
        src/log.cc
        src/dtb.cc
        src/deviceselect.cc
        src/mainwindow.cc)

message("-- Cloning OpenOCD")

execute_process(
        COMMAND git clone --recursive ${OPENOCD_URL} -b master
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

execute_process(
        COMMAND git pull
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/openocd)

message("-- Cloning DTC")

execute_process(
        COMMAND git clone --recursive ${DTC_URL} -b master
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

execute_process(
        COMMAND git pull
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dtc)

file(COPY scripts DESTINATION ${CMAKE_BINARY_DIR})
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

add_custom_target(openocd ALL
        COMMAND ./bootstrap nosubmodule
        COMMAND ./configure --enable-target64 --enable-libftdi --prefix=/
        COMMAND make
        COMMAND make install DESTDIR=${CMAKE_BINARY_DIR}/tools
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/openocd)

add_custom_target(dtc ALL
        COMMAND make
        COMMAND make install PREFIX=/usr DESTDIR=${CMAKE_BINARY_DIR}/tools
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dtc)

target_link_libraries(devclient ${GIOMM_LIBRARIES} ${GTKMM_LIBRARIES} ${LIBFTDI_LIBRARIES} fmt)
target_link_libraries(devclient pthread stdc++fs)

install(TARGETS devclient DESTINATION bin)
install(DIRECTORY ${CMAKE_BINARY_DIR}/tools/ DESTINATION tools USE_SOURCE_PERMISSIONS)
install(DIRECTORY ${CMAKE_BINARY_DIR}/scripts/ DESTINATION scripts USE_SOURCE_PERMISSIONS)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jakub Klama")
set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/conclusive/devclient")

include(CPack)
