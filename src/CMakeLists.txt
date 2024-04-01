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
            usbdevice.cpp
            wxstream.cpp)

set(HEADERS align.h
            dabmuxscanner.h
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
            scalar_condition.h
            signalmeter.h
            tcpdevice.h
            uecp.h
            usbdevice.h
            wxstream.h)

add_library(code_src OBJECT ${SOURCES} ${HEADERS})
target_link_libraries(code_src ${DEPLIBS})
target_include_directories(code_src PRIVATE ${PROJECT_SOURCE_DIR}/src)
