cmake_minimum_required(VERSION 3.5)
project(libusb)

set(PLATFORM_SRC "")
if(WIN32)
  set(PLATFORM_SRC libusb/os/events_windows.h
                   libusb/os/events_windows.c
                   libusb/os/threads_windows.h
                   libusb/os/threads_windows.c)
  set(OS_SRC msvc/config.h
             libusb/os/windows_common.h
             libusb/os/windows_common.c
             libusb/os/windows_usbdk.h
             libusb/os/windows_usbdk.c
             libusb/os/windows_winusb.h
             libusb/os/windows_winusb.c)
  include_directories(msvc)
endif()

include_directories(libusb)

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "$")
set(exec_prefix "${exec_prefix}{")
set(exec_prefix "${exec_prefix}prefix")
set(exec_prefix "${exec_prefix}}")

set(libdir "$")
set(libdir "${libdir}{")
set(libdir "${libdir}exec_prefix")
set(libdir "${libdir}}/lib")

set(includedir "$")
set(includedir "${includedir}{")
set(includedir "${includedir}prefix")
set(includedir "${includedir}}/include")

configure_file(libusb-1.0.pc.in ${PROJECT_SOURCE_DIR}/libusb-1.0.pc)

set(SOURCES libusb/libusbi.h
            libusb/version.h
            libusb/version_nano.h
            libusb/core.c
            libusb/descriptor.c
            libusb/hotplug.c
            libusb/io.c
            libusb/strerror.c
            libusb/sync.c
            ${PLATFORM_SRC}
            ${OS_SRC})

add_library(libusb-1.0 ${SOURCES})

install(TARGETS libusb-1.0 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES libusb/libusb.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libusb-1.0)
install(FILES ${PROJECT_SOURCE_DIR}/libusb-1.0.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

