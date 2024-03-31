set(SOURCES anglepch.cpp
            channeladd.cpp
            channelsettings.cpp
            dabmuxscanner.cpp
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
            stdafx.cpp
            sqlite_exception.cpp
            tcpdevice.cpp
            uecp.cpp
            wxstream.cpp)

set(HEADERS align.h
            anglepch.h
            dabmuxscanner.h
            dabstream.h
            channeladd.h
            channelsettings.h
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
            renderingcontrol.h
            rtldevice.h
            scalar_condition.h
            signalmeter.h
            stdafx.h
            socket_exception.h
            sqlite_exception.h
            string_exception.h
            tcpdevice.h
            uecp.h
            wxstream.h)

if(CORE_SYSTEM_NAME STREQUAL linux OR CORE_SYSTEM_NAME STREQUAL windows OR CORE_SYSTEM_NAME STREQUAL osx)
  list(APPEND SOURCES libusb_exception.cpp
                      usbdevice.cpp)
  list(APPEND HEADERS libusb_exception.h
                      usbdevice.h)
endif()

if(WIN32)
  list(APPEND SOURCES win32_exception.cpp)
  list(APPEND HEADERS win32_exception.h)
endif()

add_library(code_src OBJECT ${SOURCES} ${HEADERS})
target_link_libraries(code_src ${DEPLIBS})

