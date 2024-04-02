set(SOURCES demodulator.cpp
            downconvert.cpp
            fastfir.cpp
            fft.cpp
            fir.cpp
            fmdemod.cpp
            fractresampler.cpp
            iir.cpp
            wfmdemod.cpp)

set(HEADERS datatypes.h
            demodulator.h
            downconvert.h
            fastfir.h
            fft.h
            filtercoef.h
            fir.h
            fmdemod.h
            fractresampler.h
            iir.h
            rbdsconstants.h
            wfmdemod.h)

add_library(code_src_dsp_fm OBJECT ${SOURCES} ${HEADERS})
