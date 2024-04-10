/*
 *  Copyright (C) 2024 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

/*
 * Adapted partly from welle.io
 * https://github.com/AlbrechtL/welle.io/
 *
 * src/backend/pad_decoder.h
 */

#pragma once

#include "pad_decoder_datagroup.h"

// --- CMOTDecoder -----------------------------------------------------------------
class CMOTDecoder : public CDataGroup
{
public:
  CMOTDecoder();

  void Reset();

  void SetLen(size_t mot_len) { m_motLength = mot_len; }

  std::vector<uint8_t> GetMOTDataGroup();

private:
  size_t m_motLength;

  size_t GetInitialNeededSize() override { return m_motLength; } // MOT len + CRC (or zero!)
  bool DecodeDataGroup() override;
};
