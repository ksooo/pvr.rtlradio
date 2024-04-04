set(SOURCES charsets.cpp
            complex.cpp)

set(HEADERS align.h
            charsets.h
            scalar_condition.h
            value_size_defines.h)

add_library(code_src_utils OBJECT ${SOURCES} ${HEADERS})
