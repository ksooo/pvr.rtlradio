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

#include "mot_manager.h"

#include "dsp_dab/tools.h"
#include "utils/log.h"

#include <iostream>
#include <string>

// --- CMOTManager -----------------------------------------------------------------
void CMOTManager::Reset()
{
  m_object = CMOTObject();
  m_currentTransportId = -1;
}

bool CMOTManager::ParseCheckDataGroupHeader(const uint8_t* dg, 
                                            size_t size,
                                            size_t& offset,
                                            int& dg_type)
{
  // parse/check Data Group header
  if (!dg || size < (offset + 2))
    return false;

  bool extension_flag = dg[offset] & 0x80;
  bool crc_flag = dg[offset] & 0x40;
  bool segment_flag = dg[offset] & 0x20;
  bool user_access_flag = dg[offset] & 0x10;
  dg_type = dg[offset] & 0x0F;
  offset += 2 + (extension_flag ? 2 : 0);

  if (!crc_flag)
    return false;
  if (!segment_flag)
    return false;
  if (!user_access_flag)
    return false;
  if (dg_type != 3 && dg_type != 4) // only accept MOT header/body
    return false;

  return true;
}

bool CMOTManager::ParseCheckSessionHeader(const uint8_t* dg,
                                          size_t size, 
                                          size_t& offset,
                                          bool& last_seg,
                                          int& seg_number,
                                          int& transport_id)
{
  // parse/check session header
  if (!dg || size < (offset + 3))
    return false;

  last_seg = dg[offset] & 0x80;
  seg_number = ((dg[offset] & 0x7F) << 8) | dg[offset + 1];
  bool transport_id_flag = dg[offset + 2] & 0x10;
  size_t len_indicator = dg[offset + 2] & 0x0F;
  offset += 3;

  if (!transport_id_flag)
    return false;
  if (len_indicator < 2)
    return false;

  // handle transport ID
  if (size < (offset + len_indicator))
    return false;

  transport_id = (dg[offset] << 8) | dg[offset + 1];
  offset += len_indicator;

  return true;
}

bool CMOTManager::ParseCheckSegmentationHeader(const uint8_t* dg, 
                                               size_t size,
                                               size_t& offset,
                                               size_t& seg_size)
{
  // parse/check segmentation header (MOT)
  if (!dg || size < (offset + 2))
    return false;

  seg_size = ((dg[offset] & 0x1F) << 8) | dg[offset + 1];
  offset += 2;

  // compare announced/actual segment size
  if (seg_size != size - offset - CalcCRC::CRCLen)
    return false;

  return true;
}

bool CMOTManager::HandleMOTDataGroup(const uint8_t* dg, size_t size)
{
  size_t offset = 0;

  // parse/check headers
  int dg_type;
  bool last_seg;
  int seg_number;
  int transport_id;
  size_t seg_size;

  if (!ParseCheckDataGroupHeader(dg, size, offset, dg_type))
    return false;
  if (!ParseCheckSessionHeader(dg, size, offset, last_seg, seg_number, transport_id))
    return false;
  if (!ParseCheckSegmentationHeader(dg, size, offset, seg_size))
    return false;

  // add segment to MOT object (reset if necessary)
  if (m_currentTransportId != transport_id)
  {
    m_currentTransportId = transport_id;
    m_object = CMOTObject();
  }
  m_object.AddSeg(dg_type == 3, seg_number, last_seg, &dg[offset], seg_size);

  // check if object shall be shown
  bool display = m_object.IsToBeShown();
  //	fprintf(stderr, "dg_type: %d, seg_number: %2d%s, transport_id: %5d, size: %4zu; display: %s\n",
  //			dg_type, seg_number, last_seg ? " (LAST)" : "", transport_id, seg_size, display ? "true" : "false");

  // if object shall be shown, update it
  return display;
}
