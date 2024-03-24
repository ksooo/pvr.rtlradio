cmake_minimum_required(VERSION 3.5)
project(libusb)

include(ExternalProject)

list(APPEND libusb_conf CPPFLAGS=-I${CMAKE_INSTALL_PREFIX}/include)
list(APPEND libusb_conf CFLAGS=-I${CMAKE_INSTALL_PREFIX}/include)
list(APPEND libusb_conf LDFLAGS=-L${CMAKE_INSTALL_PREFIX}/lib)

ExternalProject_Add(
    libusb
    SOURCE_DIR          ${CMAKE_SOURCE_DIR}
    CONFIGURE_COMMAND   <SOURCE_DIR>/autogen.sh
                            --prefix=${OUTPUT_DIR}
                            --enable-static
                            --disable-shared
                            --disable-examples-build
                            --disable-tests-build
                            --with-pic
                            ${libusb_conf}
    INSTALL_COMMAND     ""
    BUILD_IN_SOURCE     1
)

install(CODE "execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
