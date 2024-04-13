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

#include "mot_entity.h"

// --- CMOTEntity -----------------------------------------------------------------
void CMOTEntity::AddSeg(int seg_number, bool last_seg, const uint8_t* data, size_t len)
{
  if (last_seg)
    last_seg_number = seg_number;

  if (segs.find(seg_number) != segs.end())
    return;

  // copy data
  segs[seg_number] = seg_t(len);
  memcpy(&segs[seg_number][0], data, len);
  size += len;
}

bool CMOTEntity::IsFinished()
{
  if (last_seg_number == -1)
    return false;

  // check if all segments are available
  for (int i = 0; i <= last_seg_number; i++)
    if (segs.find(i) == segs.end())
      return false;
  return true;
}

std::vector<uint8_t> CMOTEntity::GetData()
{
  std::vector<uint8_t> result(size);
  size_t offset = 0;

  // concatenate all segments
  for (int i = 0; i <= last_seg_number; i++)
  {
    seg_t& seg = segs[i];
    memcpy(&result[offset], &seg[0], seg.size());
    offset += seg.size();
  }

  return result;
}
