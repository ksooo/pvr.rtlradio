set(SOURCES fft.cpp
            Xtan2.cpp
            channels.cpp
            dab-constants.cpp
            fib-processor.cpp
            fic-handler.cpp
            freq-interleaver.cpp
            msc-handler.cpp
            ofdm-decoder.cpp
            ofdm-processor.cpp
            phasereference.cpp
            phasetable.cpp
            profiling.cpp
            protTables.cpp
            radio-receiver.cpp
            tii-decoder.cpp
            tools.cpp
            viterbi.cpp)

set(HEADERS MathHelper.h
            Xtan2.h
            channels.h
            dab-constants.h
            fft.h
            fib-processor.h
            fic-handler.h
            freq-interleaver.h
            msc-handler.h
            ofdm-decoder.h
            ofdm-processor.h
            phasereference.h
            phasetable.h
            profiling.h
            protTables.h
            radio-controller.h
            radio-receiver-options.h
            radio-receiver.h
            ringbuffer.h
            tii-decoder.h
            tools.h
            viterbi.h)

add_library(code_src_dsp_dab OBJECT ${SOURCES} ${HEADERS})
target_include_directories(code_src_dsp_dab PRIVATE ${PROJECT_SOURCE_DIR}/src)
