set(SOURCES libusb_exception.cpp
            sqlite_exception.cpp)

set(HEADERS libusb_exception.h
            socket_exception.h
            sqlite_exception.h
            string_exception.h)

if(WIN32)
  list(APPEND SOURCES win32_exception.cpp)
  list(APPEND HEADERS win32_exception.h)
endif()

add_library(code_src_exception_control OBJECT ${SOURCES} ${HEADERS})
target_include_directories(code_src_exception_control PRIVATE ${PROJECT_SOURCE_DIR}/src)