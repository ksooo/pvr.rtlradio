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

#include <cstdint>
#include <vector>

// --- CDataGroup -----------------------------------------------------------------
class CDataGroup
{
public:
  CDataGroup(size_t dg_size_max);
  virtual ~CDataGroup() = default;

  virtual bool ProcessDataSubfield(bool start, const uint8_t* data, size_t len);

protected:
  std::vector<uint8_t> m_dgRaw;
  size_t m_dgSize;
  size_t m_dgSizeNeeded;

  virtual size_t GetInitialNeededSize() { return 0; }
  virtual bool DecodeDataGroup() = 0;
  bool EnsureDataGroupSize(size_t desired_dg_size);
  bool CheckCRC(size_t len);
  void Reset();
};
