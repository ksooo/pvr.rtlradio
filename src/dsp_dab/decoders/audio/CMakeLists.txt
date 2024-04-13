set(SOURCES dab_audio.cpp)

set(HEADERS dab_audio.h
            subchannel_sink.h)

add_library(code_src_dsp_dab_decoders_audio OBJECT ${SOURCES} ${HEADERS})
target_include_directories(code_src_dsp_dab_decoders_audio PRIVATE ${PROJECT_SOURCE_DIR}/src)
