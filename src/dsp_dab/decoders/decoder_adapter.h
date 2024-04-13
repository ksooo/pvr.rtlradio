/*
 *    Copyright (C) 2018
 *    Albrecht Lohofener (albrechtloh@gmx.de)
 *
 *    This file is part of the welle.io.
 *    Many of the ideas as implemented in welle.io are derived from
 *    other work, made available through the GNU general Public License.
 *    All copyrights of the original authors are recognized.
 *
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    welle.io is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with welle.io; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef DECODER_ADAPTER_H
#define DECODER_ADAPTER_H

#include "dsp_dab/radio-controller.h"
#include "dsp_dab/decoders/dab_processor.h"
#include "dsp_dab/decoders/audio/subchannel_sink.h"
#include "dsp_dab/decoders/audio/aac/aac_decoder.h"
#include "dsp_dab/decoders/audio/mp2/mp2_decoder.h"
#include "dsp_dab/decoders/data/pad/pad_decoder.h"

#include <memory>
#include <vector>
#include <cstdint>
#include <cmath>
#include <cstdio>

class DecoderAdapter: public DabProcessor, public SubchannelSinkObserver, public IPADDecoderObserver
{
    public:
        DecoderAdapter(ProgrammeHandlerInterface& mr,
                     int16_t bitRate,
                     AudioServiceComponentType &dabModus,
                     const std::string& dumpFileName);

        virtual void addtoFrame(uint8_t *v);

        // SubchannelSinkObserver impl
        virtual void FormatChange(const AUDIO_SERVICE_FORMAT& /*format*/);
        virtual void StartAudio(int /*samplerate*/, int /*channels*/, bool /*float32*/);
        virtual void PutAudio(const uint8_t* /*data*/, size_t /*len*/);
        virtual void ProcessPAD(const uint8_t* /*xpad_data*/, size_t /*xpad_len*/, bool /*exact_xpad_len*/, const uint8_t* /*fpad_data*/);
        virtual void AudioError(const std::string& /*hint*/);
        virtual void ACCFrameError(const unsigned char /* error*/);
        virtual void FECInfo(int /*total_corr_count*/, bool /*uncorr_errors*/);

        // IPADDecoderObserver impl
        virtual void PADChangeDynamicLabel(const CDynamicLabel& dl);
        virtual void PADChangeSlide(const std::shared_ptr<MOT_FILE>& slide);
        virtual void PADLengthError(size_t announced_xpad_len, size_t xpad_len);

    private:
        int16_t bitRate;
        int frameErrorCounter = 0;
        ProgrammeHandlerInterface& myInterface;
        std::unique_ptr<SubchannelSink> decoder;
        CPADDecoder padDecoder;

        struct FILEDeleter{ void operator()(FILE* fd){ if (fd) fclose(fd); }};
        std::unique_ptr<FILE, FILEDeleter> dumpFile;

        int audioSamplerate = 0;
        int audioChannels = 0;
        std::string audioFormat;
};
#endif // DECODER_ADAPTER_H

