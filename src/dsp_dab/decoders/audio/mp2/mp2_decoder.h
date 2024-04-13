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
 * src/backend/dab_decoder.h
 */

#pragma once

#include "dsp_dab/tools.h"
#include "dsp_dab/decoders/audio/subchannel_sink.h"

#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#define MPG123_NO_LARGENAME // disable large file API here
#include "mpg123.h"
#if MPG123_API_VERSION < 36
#error "At least version 1.14.0 (API version 36) of mpg123 is required!"
#endif

// --- MP2Decoder -----------------------------------------------------------------
class MP2Decoder : public SubchannelSink
{
public:
  MP2Decoder(SubchannelSinkObserver* observer, bool float32);
  ~MP2Decoder();

  void Feed(const uint8_t* data, size_t len);

private:
  bool float32;
  mpg123_handle* handle;

  int scf_crc_len;
  bool lsf;
  std::vector<uint8_t> frame;

  void ProcessFormat();
  void ProcessUntouchedStream(const unsigned long& header,
                              const uint8_t* body_data,
                              size_t body_bytes);
  size_t DecodeFrame(uint8_t** data);
  bool CheckCRC(const unsigned long& header, const uint8_t* body_data, const size_t& body_bytes);

  static const int table_nbal_48a[];
  static const int table_nbal_48b[];
  static const int table_nbal_24[];
  static const int* tables_nbal[];
  static const int sblimits[];
};
