set(SOURCES pad_decoder.cpp
            pad_decoder_datagroup.cpp
            pad_decoder_dgli.cpp
            pad_decoder_dynamiclabelsegment.cpp
            pad_decoder_mot.cpp)

set(HEADERS pad_decoder.h
            pad_decoder_datagroup.h
            pad_decoder_dgli.h
            pad_decoder_dynamiclabelsegment.h
            pad_decoder_mot.h)

add_library(code_src_dsp_dab_decoders_data_pad OBJECT ${SOURCES} ${HEADERS})
target_include_directories(code_src_dsp_dab_decoders_data_pad PRIVATE ${PROJECT_SOURCE_DIR}/src)
