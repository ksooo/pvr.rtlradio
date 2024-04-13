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
 * src/backend/mot_manager.cpp
 */

#pragma once

#include "mot_file.h"
#include "mot_object.h"

#include <vector>

// --- CMOTManager -----------------------------------------------------------------
class CMOTManager
{
public:
  CMOTManager() = default;

  void Reset();
  bool HandleMOTDataGroup(const uint8_t* dg, size_t size);
  std::shared_ptr<MOT_FILE> GetFile() { return m_object.GetFile(); }

private:
  CMOTObject m_object;
  int m_currentTransportId{-1};

  bool ParseCheckDataGroupHeader(const uint8_t* dg, size_t size, size_t& offset, int& dg_type);
  bool ParseCheckSessionHeader(const uint8_t* dg,
                               size_t size,
                               size_t& offset,
                               bool& last_seg,
                               int& seg_number,
                               int& transport_id);
  bool ParseCheckSegmentationHeader(const uint8_t* dg,
                                    size_t size,
                                    size_t& offset,
                                    size_t& seg_size);
};
