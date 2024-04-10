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
#include "utils/charsets.h"

#include <memory>

// --- CDynamicLabel -----------------------------------------------------------------
class CDynamicLabel
{
public:
  CDynamicLabel() = default;

  void Reset()
  {
    raw.clear();
    charset = -1;
  }

  std::vector<uint8_t> raw;
  int charset{-1};
};

struct DL_SEG_REASSEMBLER;

// --- CDynamicLabelDecoder -----------------------------------------------------------------
class CDynamicLabelDecoder : public CDataGroup
{
public:
  CDynamicLabelDecoder();
  ~CDynamicLabelDecoder();

  void Reset();

  CDynamicLabel GetLabel() { return m_label; }

private:
  size_t GetInitialNeededSize() override;
  bool DecodeDataGroup() override;

  std::unique_ptr<DL_SEG_REASSEMBLER> m_dl_sr;
  CDynamicLabel m_label;
};
