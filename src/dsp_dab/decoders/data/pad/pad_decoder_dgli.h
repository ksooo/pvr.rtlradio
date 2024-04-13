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

// --- CDGLIDecoder -----------------------------------------------------------------
class CDGLIDecoder : public CDataGroup
{
public:
  CDGLIDecoder();

  void Reset();

  size_t GetDGLILen();

private:
  size_t m_dgliLength{0};

  size_t GetInitialNeededSize() override;
  bool DecodeDataGroup() override;
};
