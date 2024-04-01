set(SOURCES complex.cpp)

set(HEADERS align.h
            scalar_condition.h
            value_size_defines.h)

add_library(code_src_utils OBJECT ${SOURCES} ${HEADERS})
