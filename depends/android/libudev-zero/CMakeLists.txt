cmake_minimum_required(VERSION 3.5)
project(libudev-zero)

set(SOURCES udev.c
            udev_device.c
            udev_enumerate.c
            udev_list.c
            udev_monitor.c)

add_library(udev ${SOURCES})

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

configure_file(libudev.pc.in ${PROJECT_SOURCE_DIR}/libudev.pc)

install(TARGETS udev DESTINATION lib)
install(FILES udev.h DESTINATION include RENAME libudev.h)
install(FILES libudev.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
