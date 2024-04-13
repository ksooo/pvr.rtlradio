/*
 *  Copyright (C) 2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Adapted from welle.io
 * https://github.com/AlbrechtL/welle.io/
 *
 * src/backend/subchannel_sink.h
 */

#pragma once

#include <cstdint>
#include <mutex>
#include <set>
#include <string>

#define FPAD_LEN 2

// --- AUDIO_SERVICE_FORMAT -----------------------------------------------------------------
struct AUDIO_SERVICE_FORMAT
{
  std::string codec;
  size_t samplerate_khz;
  std::string mode;
  size_t bitrate_kbps;

  AUDIO_SERVICE_FORMAT() : samplerate_khz(0), bitrate_kbps(0) {}
  std::string GetSummary() const
  {
    return codec + ", " + std::to_string(samplerate_khz) + " kHz " + mode + " @ " +
           std::to_string(bitrate_kbps) + " kbit/s";
  }
};

// --- SubchannelSinkObserver -----------------------------------------------------------------
class SubchannelSinkObserver
{
public:
  virtual ~SubchannelSinkObserver() {}

  virtual void FormatChange(const AUDIO_SERVICE_FORMAT& /*format*/) {}
  virtual void StartAudio(int /*samplerate*/, int /*channels*/, bool /*float32*/) {}
  virtual void PutAudio(const uint8_t* /*data*/, size_t /*len*/) {}
  virtual void ProcessPAD(const uint8_t* /*xpad_data*/,
                          size_t /*xpad_len*/,
                          bool /*exact_xpad_len*/,
                          const uint8_t* /*fpad_data*/)
  {
  }

  virtual void AudioError(const std::string& /*hint*/) {}
  virtual void ACCFrameError(const unsigned char /* error*/) {}
  virtual void FECInfo(int /*total_corr_count*/, bool /*uncorr_errors*/) {}
};

// --- UntouchedStreamConsumer -----------------------------------------------------------------
class UntouchedStreamConsumer
{
public:
  virtual ~UntouchedStreamConsumer() {}

  virtual void ProcessUntouchedStream(const uint8_t* /*data*/,
                                      size_t /*len*/,
                                      size_t /*duration_ms*/) = 0;
};

// --- SubchannelSink -----------------------------------------------------------------
class SubchannelSink
{
protected:
  SubchannelSinkObserver* observer;
  std::string untouched_stream_file_extension;

  std::mutex uscs_mutex;
  std::set<UntouchedStreamConsumer*> uscs;

  void ForwardUntouchedStream(const uint8_t* data, size_t len, size_t duration_ms)
  {
    // mutex must already be locked!
    for (UntouchedStreamConsumer* usc : uscs)
      usc->ProcessUntouchedStream(data, len, duration_ms);
  }

public:
  SubchannelSink(SubchannelSinkObserver* observer, std::string untouched_stream_file_extension)
    : observer(observer), untouched_stream_file_extension(untouched_stream_file_extension)
  {
  }
  virtual ~SubchannelSink() {}

  virtual void Feed(const uint8_t* data, size_t len) = 0;
  std::string GetUntouchedStreamFileExtension() { return untouched_stream_file_extension; }
  void AddUntouchedStreamConsumer(UntouchedStreamConsumer* consumer)
  {
    std::lock_guard<std::mutex> lock(uscs_mutex);
    uscs.insert(consumer);
  }
  void RemoveUntouchedStreamConsumer(UntouchedStreamConsumer* consumer)
  {
    std::lock_guard<std::mutex> lock(uscs_mutex);
    uscs.erase(consumer);
  }
};
