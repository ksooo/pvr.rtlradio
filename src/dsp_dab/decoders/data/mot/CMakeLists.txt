set(SOURCES mot_entity.cpp
            mot_file.cpp
            mot_manager.cpp
            mot_object.cpp)

set(HEADERS mot_entity.h
            mot_file.h
            mot_manager.h
            mot_object.h)

add_library(code_src_dsp_dab_decoders_data_mot OBJECT ${SOURCES} ${HEADERS})
target_include_directories(code_src_dsp_dab_decoders_data_mot PRIVATE ${PROJECT_SOURCE_DIR}/src)
