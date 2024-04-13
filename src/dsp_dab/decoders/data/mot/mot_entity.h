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

#include <map>
#include <vector>

typedef std::vector<uint8_t> seg_t;
typedef std::map<int, seg_t> segs_t;

// --- CMOTEntity -----------------------------------------------------------------
class CMOTEntity
{
public:
  CMOTEntity() = default;
  void Reset()
  {
    segs.clear();
    last_seg_number = -1;
    size = 0;
  }

  void AddSeg(int seg_number, bool last_seg, const uint8_t* data, size_t len);
  bool IsFinished();
  size_t GetSize() { return size; }
  std::vector<uint8_t> GetData();

private:
  segs_t segs;
  int last_seg_number{-1};
  size_t size{0};
};
