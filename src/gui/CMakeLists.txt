set(SOURCES channeladd.cpp
            channelscan.cpp
            channelsettings.cpp)

set(HEADERS channeladd.h
            channelscan.h
            channelsettings.h
            renderingcontrol.h)

add_library(code_src_gui OBJECT ${SOURCES} ${HEADERS})
target_include_directories(code_src_gui PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(code_src_gui ${DEPLIBS})
