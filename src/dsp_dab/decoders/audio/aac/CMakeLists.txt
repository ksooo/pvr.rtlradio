set(SOURCES aac_decoder.cpp
            fec.c)

set(HEADERS aac_decoder.h
            fec.h)

add_library(code_src_dsp_dab_decoders_audio_aac OBJECT ${SOURCES} ${HEADERS})
target_include_directories(code_src_dsp_dab_decoders_audio_aac PRIVATE ${PROJECT_SOURCE_DIR}/src)
