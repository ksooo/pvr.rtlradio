set(SOURCES sqlite_exception.cpp)

set(HEADERS socket_exception.h
            sqlite_exception.h
            string_exception.h)

if(USB_DEVICE_SUPPORT)
  list(APPEND SOURCES libusb_exception.cpp)
  list(APPEND HEADERS libusb_exception.h)
endif()

if(WIN32)
  list(APPEND SOURCES win32_exception.cpp)
  list(APPEND HEADERS win32_exception.h)
endif()

add_library(code_src_exception_control OBJECT ${SOURCES} ${HEADERS})
target_include_directories(code_src_exception_control PRIVATE ${PROJECT_SOURCE_DIR}/src)