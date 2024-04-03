set(SOURCES dabmuxscanner.cpp
            dabstream.cpp
            database.cpp
            filedevice.cpp
            fmstream.cpp
            hdmuxscanner.cpp
            hdstream.cpp
            id3v1tag.cpp
            id3v2tag.cpp
            rdsdecoder.cpp
            signalmeter.cpp
            tcpdevice.cpp
            uecp.cpp
            wxstream.cpp)

set(HEADERS dabmuxscanner.h
            dabstream.h
            database.h
            filedevice.h
            fmstream.h
            hdmuxscanner.h
            hdstream.h
            id3v1tag.h
            id3v2tag.h
            dbtypes.h
            muxscanner.h
            props.h
            pvrstream.h
            pvrtypes.h
            rdsdecoder.h
            rtldevice.h
            signalmeter.h
            tcpdevice.h
            uecp.h
            wxstream.h)

if(USB_DEVICE_SUPPORT)
  list(APPEND SOURCES usbdevice.cpp)
  list(APPEND HEADERS usbdevice.h)
endif()

add_library(code_src OBJECT ${SOURCES} ${HEADERS})
target_link_libraries(code_src ${DEPLIBS})
target_include_directories(code_src PRIVATE ${PROJECT_SOURCE_DIR}/src)
