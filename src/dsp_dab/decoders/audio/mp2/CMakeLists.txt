set(SOURCES mp2_decoder.cpp)

set(HEADERS mp2_decoder.h)

add_library(code_src_dsp_dab_decoders_audio_mp2 OBJECT ${SOURCES} ${HEADERS})
target_include_directories(code_src_dsp_dab_decoders_audio_mp2 PRIVATE ${PROJECT_SOURCE_DIR}/src)
