set(SOURCES acquire.c
            conv_dec.c
            decode.c
            firdecim_q15.c
            frame.c
            input.c
            nrsc5.c
            output.c
            pids.c
            rs_decode.c
            rs_init.c
            strndup.c
            sync.c
            unicode.c)

set(HEADERS acquire.h
            bitwriter.h
            config.h
            conv.h
            conv_gen.h
            conv_neon.h
            conv_sse.h
            decode.h
            defines.h
            firdecim_q15.h
            frame.h
            input.h
            nrsc5.h
            output.h
            pids.h
            private.h
            rs_char.h
            sync.h
            unicode.h)

add_library(code_src_dsp_hd OBJECT ${SOURCES} ${HEADERS})
