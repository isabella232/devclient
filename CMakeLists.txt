cmake_minimum_required(VERSION 3.15)
project(devclient)
set(CMAKE_CXX_STANDARD 17)

set(OPENOCD_URL https://github.com/conclusiveeng/openocd.git)
set(DTC_URL git://git.kernel.org/pub/scm/utils/dtc/dtc.git)

set(OPENOCD_VER samthedongle)
set(DTC_VER v1.5.1)

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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/contrib/filesystem-1.2.10/include)
include_directories(include)

set(SOURCES
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

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_executable(devclient MACOSX_BUNDLE ${SOURCES})
else()
    add_executable(devclient ${SOURCES})
endif()

message("-- Cloning OpenOCD")

execute_process(
        COMMAND git clone --recursive ${OPENOCD_URL} -b ${OPENOCD_VER}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

execute_process(
        COMMAND git pull
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/openocd)

message("-- Cloning DTC")

execute_process(
        COMMAND git clone --recursive ${DTC_URL} -b ${DTC_VER}
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
        COMMAND make install PREFIX=/usr/local DESTDIR=${CMAKE_BINARY_DIR}/tools
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dtc)

target_link_libraries(devclient ${GIOMM_LIBRARIES} ${GTKMM_LIBRARIES} ${LIBFTDI_LIBRARIES} fmt)
target_link_libraries(devclient pthread)

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_libraries(devclient stdc++fs)
endif()

install(TARGETS devclient
        BUNDLE DESTINATION .
        RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_BINARY_DIR}/tools/
        DESTINATION tools
        USE_SOURCE_PERMISSIONS)
install(DIRECTORY ${CMAKE_BINARY_DIR}/scripts/
        DESTINATION scripts
        USE_SOURCE_PERMISSIONS)

set(CPACK_PACKAGE_VENDOR "Conclusive Engineering")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_CHECKSUM SHA256)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CPACK_GENERATOR DragNDrop)
    set(CPACK_BUNDLE_NAME Devclient)
    set(CPACK_BUNDLE_PLIST ${CMAKE_BINARY_DIR}/../package/macos/Info.plist)
    set(CPACK_BUNDLE_ICON ${CMAKE_BINARY_DIR}/../package/macos/icon.png)

    set(MACOS_APP "${CMAKE_BINARY_DIR}/Devclient.app")
    set(MACOS_APP_LIBS ${CMAKE_BINARY_DIR}/tools/usr/local/lib)
    install(CODE
            "include(BundleUtilities)
            fixup_bundle(${MACOS_APP} ${MACOS_APP_LIBS} \"\")")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jakub Klama")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/conclusive/devclient")
else()
    message("-- Package generation unsupported for your platform")
endif()

include(CPack)
