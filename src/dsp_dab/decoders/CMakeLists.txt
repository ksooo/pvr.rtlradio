set(SOURCES decoder_adapter.cpp
            eep_protection.cpp
            uep_protection.cpp)

set(HEADERS dab_processor.h
            dab_virtual.h
            decoder_adapter.h
            eep_protection.h
            energy_dispersal.h
            protection.h
            uep_protection.h)

add_library(code_src_dsp_dab_decoders OBJECT ${SOURCES} ${HEADERS})
target_include_directories(code_src_dsp_dab_decoders PRIVATE ${PROJECT_SOURCE_DIR}/src)
